namespace uCNCgui;
using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Security.Principal;
using System.Text;
using System.Threading;
using System.Runtime.InteropServices;

public partial class Main : Form
{
    [StructLayout(LayoutKind.Sequential)]
    public struct VirtualMap_T
    {
        public System.UInt32 Outputs;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public System.Byte[] PWM;
        public System.UInt32 Inputs;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public System.Byte[] Analogs;
    }

    public Main()
    {
        InitializeComponent();
    }

    delegate void OutputCheckBoxCallback(string id, bool value);

    void OutputCheckBox(string id, bool value)
    {
        Control[] c = this.Controls.Find(id, true);
        if (c.Length != 0)
        {
            CheckBox chk = (CheckBox)c[0];
            if (chk.InvokeRequired)
            {
                OutputCheckBoxCallback d = new OutputCheckBoxCallback(OutputCheckBox);
                this.Invoke(d, new object[] { id, value });
            }
            else
            {
                chk.Checked = value;
            }
            
        }
    }

    delegate void OutputProgressBarCallback(string id, byte value);

    void OutputProgressBar(string id, byte value)
    {
        Control[] c = this.Controls.Find(id, true);
        if (c.Length != 0)
        {
            ProgressBar pgr = (ProgressBar)c[0];
            if (pgr.InvokeRequired)
            {
                OutputProgressBarCallback d = new OutputProgressBarCallback(OutputProgressBar);
                this.Invoke(d, new object[] { id, value });
            }
            else
            {
                pgr.Value = (int)value;
            }

        }
    }

    delegate void StatusInfoTextCallback(ToolStripStatusLabel c, string value);
    void StatusInfoText(ToolStripStatusLabel c, string value)
    {
        //if (c.InvokeRequired)
        //{
        //    StatusInfoTextCallback d = new StatusInfoTextCallback(StatusInfoText);
        //    this.Invoke(d, new object[] { c, value });
        //}
        //else
        //{
            c.Text = value;
        //}
    }

    private long _lastTimeStamp = 0;
    private bool _quit = false;
    private void pipeClient_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
    {
        while (!e.Cancel && !_quit)
        {
            StatusInfoText(connectionStatus, "Disconnected");
            StatusInfoText(connectionFreq, "Update frequency: N/A");

            var pipeClient =
                    new NamedPipeClientStream(".", "ucncio",
                        PipeDirection.InOut, PipeOptions.None,
                        TokenImpersonationLevel.None);

            StatusInfoText(connectionStatus, "Waiting for client");

            try
            {
                pipeClient.Connect(1000);

                StatusInfoText(connectionStatus, "Connected");

                Stream pipeStream = pipeClient;

                // Validate the server's signature string.
                while (!e.Cancel && !_quit)
                {
                    _lastTimeStamp = DateTime.Now.Ticks;
                    int len;
                    len = 40;
                    byte[] buffer = new byte[len];
                    pipeStream.Read(buffer, 0, len);
                    GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                    VirtualMap_T pins;
                    try
                    {
                        pins = (VirtualMap_T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(VirtualMap_T));
                    }
                    finally
                    {
                        handle.Free();
                    }

                    //update ouputs
                    for (int i = 0; i < 32; i++)
                    {
                        int index = i;
                        if (i > 19)
                        {
                            index += 16;
                        }

                        bool value = ((pins.Outputs & (1 << i)) != 0);
                        OutputCheckBox("d" + index, value);
                    }

                    //update PWM
                    for (int i = 0; i < 16; i++)
                    {
                        byte value = pins.PWM[i];
                        OutputProgressBar("d" + (i + 20), value);
                    }

                    //read inputs
                    for (int i = 0; i < 32; i++)
                    {
                        int index = i;
                        if (i > 13)
                        {
                            index += 16;
                        }
                        Control[] c = this.Controls.Find("d" + (52 + index), true);
                        if (c.Length != 0)
                        {
                            CheckBox chk = (CheckBox)c[0];
                            bool value = chk.Checked;
                            if (value)
                            {
                                pins.Inputs |= (1U << i);
                            }
                            else
                            {
                                pins.Inputs &= ~(1U << i);
                            }
                        }
                    }

                    //read analogs
                    for (int i = 0; i < 16; i++)
                    {
                        Control[] c = this.Controls.Find("d" + (66 + i), true);
                        if (c.Length != 0)
                        {
                            NumericUpDown num = (NumericUpDown)c[0];
                            byte value = (byte)num.Value;
                            pins.Analogs[i] = value;
                        }
                    }

                    handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                    try
                    {
                        Marshal.StructureToPtr(pins, handle.AddrOfPinnedObject(), true);
                        pipeStream.Write(buffer, 0, 40);
                        pipeStream.Flush();
                    }
                    catch
                    {
                        break;
                    }
                    finally
                    {
                        handle.Free();

                    }

                    long current = DateTime.Now.Ticks - _lastTimeStamp;

                    int freq = (int)Math.Round(1000.0 / TimeSpan.FromTicks(current).TotalMilliseconds);
                    StatusInfoText(connectionFreq, "Update frequency: " + freq + "Hz");
                }

                pipeStream.Close();
                pipeClient.Close();
            }
            catch
            {

            }
        }
    }

    private void Main_Load(object sender, EventArgs e)
    {
        pipeClient.RunWorkerAsync();
    }

    private void Main_FormClosing(object sender, FormClosingEventArgs e)
    {
        pipeClient.CancelAsync();
        _quit = true;
        while (pipeClient.CancellationPending)
        {
            Application.DoEvents();
        }   
    }
}
