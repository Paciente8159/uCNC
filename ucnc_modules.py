import os
import requests
import zipfile
import io
import shutil

from pathlib import Path

Import("env")  # Import the PlatformIO build environment

# Local path where the zip file will be temporarily saved
TEMP_ZIP_PATH = "tmp/modules.zip"
SRC_DIR = env.subst("$PROJECT_SRC_DIR")
UCNC_MODULES_PATH = os.path.join(SRC_DIR, "src", "modules")


def download_file(url, local_path):
    print(f"Downloading modules from {url}")

    # Perform the HTTP GET request
    response = requests.get(url)
    response.raise_for_status()

    # Ensure the directory exists
    os.makedirs(os.path.dirname(local_path), exist_ok=True)

    # Write the zip content to a file
    with open(local_path, "wb") as file:
        file.write(response.content)

    print(f"File saved to {local_path}")


def extract_folders_from_zip(zip_path, extract_path, folders):
    print(f"Extracting modules: {folders} from {zip_path}")

    # Ensure the extraction directory exists
    os.makedirs(extract_path, exist_ok=True)

    with zipfile.ZipFile(zip_path, "r") as zip_ref:
        # Iterate through all the files in the zip file
        rootdir = zip_ref.infolist()[0]
        for file_info in zip_ref.infolist():
            # Extract only the files that are in the specified folders
            for folder in folders:
                if file_info.filename.upper().startswith(
                    f"{rootdir.filename.upper()}{folder.upper()}/"
                ):
                    file_name = file_info.filename.split("/")[-1]
                    dest_path = os.path.join(extract_path, folder)
                    if file_info.is_dir():
                        continue
                    file_info.filename = os.path.relpath(
                        file_info.filename, os.path.join(rootdir.filename, folder)
                    )
                    if os.path.exists(os.path.join(dest_path, file_info.filename)):
                        continue
                    print(f"Extracting {file_info.filename} to {dest_path}")
                    zip_ref.extract(file_info, dest_path)

    print(f"Modules extracted to {extract_path}")


def pre_getmodules_action():
    extract_path = Path(UCNC_MODULES_PATH)
    # Fetch the custom URL and folders from platformio.ini environment
    modules_url = env.GetProjectOption("custom_ucnc_modules_url")
    extract_modules = env.GetProjectOption("custom_ucnc_modules")

    if not modules_url:
        return

    if not extract_modules:
        print(
            "No µCNC modules are defined in the platformio.ini file and will not be added"
        )
        return

    # if extract_path.exists() and extract_path.is_dir():
    #     print(f"Removing modules from {UCNC_MODULES_PATH}")

    #     for folder in extract_path.iterdir():
    #         # Skip the "languages" folder
    #         if folder.name != "language" and folder.is_dir():
    #             # Remove the entire extracted_data directory
    #             print(f"Delete {folder}")
    #             shutil.rmtree(str(folder))

    #     print(f"Modules removed.")
    # else:
    #     print(f"No modules removed from {UCNC_MODULES_PATH}")

    # Split the extract_modules by comma and remove any extra spaces
    modules_to_extract = [
        folder.strip() for folder in ",".join(extract_modules.splitlines()).split(",")
    ]

    # Download the zip file
    download_file(modules_url, TEMP_ZIP_PATH)

    # Extract specified folders from the zip file
    extract_folders_from_zip(TEMP_ZIP_PATH, UCNC_MODULES_PATH, modules_to_extract)

    # Clean up: Optionally remove the zip file after extraction
    if os.path.exists(TEMP_ZIP_PATH):
        os.remove(TEMP_ZIP_PATH)
        print(f"Temporary zip file {TEMP_ZIP_PATH} removed.")


pre_getmodules_action()
