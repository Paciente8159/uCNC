import os
import requests
import zipfile
import io

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
    with open(local_path, 'wb') as file:
        file.write(response.content)

    print(f"File saved to {local_path}")

def extract_folders_from_zip(zip_path, extract_path, folders):
    print(f"Extracting folders: {folders} from {zip_path}")

    # Ensure the extraction directory exists
    os.makedirs(extract_path, exist_ok=True)

    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        # Iterate through all the files in the zip file
        for file_info in zip_ref.infolist():
            # Extract only the files that are in the specified folders
            for folder in folders:
                if file_info.filename.startswith(f"{folder}/"):
                    print(f"Extracting {file_info.filename}")
                    zip_ref.extract(file_info, extract_path)
    
    print(f"Folders extracted to {extract_path}")

def pre_action():
    # Fetch the custom URL and folders from platformio.ini environment
    ucnc_modules_url = env.GetProjectOption("ucnc_modules_url")
    extract_modules = env.GetProjectOption("extract_modules")
    
    if not ucnc_modules_url:
        sys.exit(0)
    
    if not extract_modules:
        print("No µCNC modules are defined in the platformio.ini file and will not be added")
        sys.exit(0)
    
    # Split the extract_modules by comma and remove any extra spaces
    modules_to_extract = [folder.strip() for folder in extract_modules.splitlines().split(",")]

    # Download the zip file
    download_file(ucnc_modules_url, TEMP_ZIP_PATH)
    
    # Extract specified folders from the zip file
    extract_folders_from_zip(TEMP_ZIP_PATH, UCNC_MODULES_PATH, modules_to_extract)

    # Clean up: Optionally remove the zip file after extraction
    if os.path.exists(TEMP_ZIP_PATH):
        os.remove(TEMP_ZIP_PATH)
        print(f"Temporary zip file {TEMP_ZIP_PATH} removed.")

if __name__ == "__main__":
    pre_action()
