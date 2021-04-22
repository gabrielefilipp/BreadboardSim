using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;


namespace SimGUI
{
    class Memory : Component
    {
        private byte[] Mem;
        private ushort Address;

        private const double RClosed = 1e-3;
        private const double RNormal = RClosed;
        private const double ROpen = 1e12;
        public Memory(Circuit parent, Point origin)
            : base(parent, origin)
        {
            ComponentType = "Memory";
            ComponentModel = "HT23C512";
            ID = parent.GetNextComponentName("MEM");

            LoadFootprintFromXml("DIP28");

            Address = 0x0;
            Mem = new byte[65536];
            Mem[0x0000] = 0xF;
            Mem[0x0001] = 0x9;
            Mem[0x001B] = 0x1E;
            /*
            int counter = 0;
            string line;
            
            System.IO.StreamReader file = new System.IO.StreamReader(@".\test.txt");
            while ((line = file.ReadLine()) != null)
            {
                System.Console.WriteLine(line);
                counter++;
            }
            file.Close();
            */
        }

        //A slight variation : we want ICs to only display their part number and not their full name
        public override void UpdateText()
        {
            base.UpdateText();
            foreach (var textObject in Children.OfType<TextBlock>())
            {
                if (textObject.Name == "_Model")
                {
                    if (ComponentModel.IndexOf(' ') != -1)
                    {
                        textObject.Text = ComponentModel.Substring(0, ComponentModel.IndexOf(' '));
                    }
                    else
                    {
                        textObject.Text = ComponentModel;
                    }
                    Util.DoEvents();
                }
            }
        }
        
        public override string GenerateNetlist()
        {
            string netlist = "";

            netlist += "LOGIC_AND " + ID + "a0 " + ConnectedNets[10] + " " + ConnectedNets[20] + " " + ID + ".A0 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a1 " + ConnectedNets[9] + " " + ConnectedNets[20] + " " + ID + ".A1 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a2 " + ConnectedNets[8] + " " + ConnectedNets[20] + " " + ID + ".A2 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a3 " + ConnectedNets[7] + " " + ConnectedNets[20] + " " + ID + ".A3 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a4 " + ConnectedNets[6] + " " + ConnectedNets[20] + " " + ID + ".A4 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a5 " + ConnectedNets[5] + " " + ConnectedNets[20] + " " + ID + ".A5 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a6 " + ConnectedNets[4] + " " + ConnectedNets[20] + " " + ID + ".A6 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a7 " + ConnectedNets[3] + " " + ConnectedNets[20] + " " + ID + ".A7 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a8 " + ConnectedNets[25] + " " + ConnectedNets[20] + " " + ID + ".A8 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a9 " + ConnectedNets[24] + " " + ConnectedNets[20] + " " + ID + ".A9 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a10 " + ConnectedNets[21] + " " + ConnectedNets[20] + " " + ID + ".A10 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a11 " + ConnectedNets[23] + " " + ConnectedNets[20] + " " + ID + ".A11 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a12 " + ConnectedNets[2] + " " + ConnectedNets[20] + " " + ID + ".A12 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a13 " + ConnectedNets[26] + " " + ConnectedNets[20] + " " + ID + ".A13 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a14 " + ConnectedNets[27] + " " + ConnectedNets[20] + " " + ID + ".A14 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            netlist += "LOGIC_AND " + ID + "a15 " + ConnectedNets[1] + " " + ConnectedNets[20] + " " + ID + ".A15 " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";

            for (int i = 0; i < 16; i++)
            {
                netlist += "RES " + ID + ".RA" + i.ToString() + " " + ID + ".A" + i.ToString() + " " + ConnectedNets[14] + " res=" + RNormal + "\r\n";
            }

            netlist += "LOGIC_AND " + ID + "di " + ConnectedNets[20] + " " + ConnectedNets[22] + " " + ID + ".DI " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";

            for (int i = 0; i < 8; i++)
            {
                netlist += "LOGIC_AND " + ID + "d" + i.ToString() + " " + ConnectedNets[28] + " " + ID + ".DI " + ID + ".D" + i.ToString() + " " + ConnectedNets[14] + " " + ConnectedNets[28] + "\r\n";
            }

            byte b = Mem[Address];
            for (int i = 0; i < 8; i++)
            {
                netlist += "RES " + ID + ".RD" + i.ToString() + " " + ID + ".D" + i.ToString() + " " + ConnectedNets[11 + (i < 3 ? i : i + 1)] + " res=" + (((b >> i) & 0x1) == 0x1 ? RClosed : ROpen) + "\r\n";
            }

            return netlist;
        }

        private void UpdateOutput(ushort Addr) {
            if (Addr != Address)
            {
                byte b = Mem[Addr];
                for (int i = 0; i < 8; i++)
                {
                    //Notify of RES changes
                    ParentCircuit.ParentWindow.CurrentSimulator.SendChangeMessage("CHANGE " + ID + ".RD" + i.ToString() + " res=" + (((b >> i) & 0x1) == 0x1 ? RClosed : ROpen));
                }
                Address = Addr;
            }
        }

        public override void UpdateFromSimulation(int numberOfUpdates, Simulator sim, SimulationEvent eventType)
        {
            base.UpdateFromSimulation(numberOfUpdates, sim, eventType);

            if (eventType == SimulationEvent.TICK)
            {
                ushort Addr = 0x0;
                for (int i = 0; i < 16; i++)
                {
                    int varID = sim.GetComponentPinCurrentVarId(ID + "a" + i.ToString(), 3);
                    if (varID != -1)
                    {
                        double PinCurrent = sim.GetValueOfVar(varID, 0);

                        if (PinCurrent != 0)
                        {
                            //SetBrightness(i, Math.Min(LEDCurrent / 0.01, 1.0));
                            Addr |= (ushort)(1 << i);
                        }
                    }
                }
                UpdateOutput(Addr);
            }
            else if (eventType == SimulationEvent.STOPPED)
            {
                UpdateOutput(0x0);
            }

        }
    }
}
