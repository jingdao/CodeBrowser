using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;

namespace FileMatcher
{
    public partial class FileMatcherForm : Form
    {
        int countDirectories = 0;
        int countFiles = 0;
        int countIncluded = 0;
        int countTokens = 0;
        int countLines = 0;
        int lastPoint = 0;
        int maxLines = 50;
        int historyIndex = 0;
        Dictionary<string, List<Tuple<string,int>>> allTokens = new Dictionary<string, List<Tuple<string,int>>>();
        Dictionary<string, List<string>> linesFromFile = new Dictionary<string, List<string>>();
        List<string> history = new List<string>();
        List<string> included = new List<string>();
        List<string> allExtensions = new List<string>();
        Stopwatch timer;
        public FileMatcherForm()
        {
            InitializeComponent();
        }

        private void findAllOccurences(string key)
        {
            timer = Stopwatch.StartNew();
            if (!allTokens.ContainsKey(key))
            {
                richTextBox1.AppendText("\r\n    no occurence of '"+key+"' found.\r\n>>");
                return;
            }
            List<Tuple<string,int>> ls = allTokens[key];
            richTextBox1.AppendText("\r\n");
            if (ls.Count > maxLines)
            {
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Green;
                richTextBox1.AppendText("    Query complete. Too many results (" + ls.Count + ") to display.\r\n");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = richTextBox1.ForeColor;
                richTextBox1.AppendText(">>");
                return;
            }
            for (int i=0;i<ls.Count;i++) {
                Tuple<string, int> entry = ls[i];
                string line = linesFromFile[entry.Item1][entry.Item2 - 1];
                int tokenIndex = -1;
                for (int j = 0; j < line.Length; j++)
                {
                    if (Char.IsLetterOrDigit(line[j]) || line[j] == '_')
                    {
                        if (tokenIndex < 0) tokenIndex = j;
                    }
                    else if (tokenIndex >= 0)
                    {
                        if (line.Substring(tokenIndex, j - tokenIndex) == key) break;
                        else tokenIndex = -1;
                    }
                }
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.DarkBlue;
                richTextBox1.AppendText(entry.Item1 + ":");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Cyan;
                richTextBox1.AppendText(entry.Item2 + " ");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = richTextBox1.ForeColor;
                richTextBox1.AppendText(line.Substring(0, tokenIndex));
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Red;
                richTextBox1.AppendText(key);
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = richTextBox1.ForeColor;
                if (tokenIndex + key.Length == line.Length) richTextBox1.AppendText("\r\n");
                else richTextBox1.AppendText(line.Substring(tokenIndex + key.Length, line.Length - (tokenIndex + key.Length)) + "\r\n");
            }
            richTextBox1.Select(richTextBox1.TextLength, 0);
            richTextBox1.SelectionColor = Color.Green;
            timer.Stop();
            richTextBox1.AppendText("    Query complete ("+timer.ElapsedMilliseconds+"ms). "+ls.Count+" occurences of '"+key+"' found.\r\n\r\n");
            richTextBox1.Select(richTextBox1.TextLength, 0);
            richTextBox1.SelectionColor = richTextBox1.ForeColor;
            richTextBox1.AppendText(">>");            
        }

        private void addToken(string token, string fileName, int lineNumber)
        {
            if (!allTokens.ContainsKey(token))
            {
                allTokens.Add(token, new List<Tuple<string, int>>());
            }
            allTokens[token].Add(new Tuple<string, int>(fileName,lineNumber));
            countTokens++;
        }

        private void parseFromFile(string fileName)
        {
            List<string> l = new List<string>();
            string line;
            string fn = fileName.Substring(comboBox1.Text.Length+1, fileName.Length - comboBox1.Text.Length - 1);
            int tokenIndex = -1, lineNumber = 0;
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            while ((line = file.ReadLine()) != null)
            {
                l.Add(line.Trim());               
                lineNumber++;
                countLines++;
                for (int i = 0; i < line.Length; i++)
                {
                    if (Char.IsLetterOrDigit(line[i]) || line[i] == '_')
                    {
                        if (tokenIndex < 0) tokenIndex = i;
                    }
                    else if (tokenIndex >= 0)
                    {
                        addToken(line.Substring(tokenIndex, i - tokenIndex), fn, lineNumber);
                        tokenIndex = -1;
                    }
                }
                if (tokenIndex >= 0) addToken(line.Substring(tokenIndex, line.Length - tokenIndex), fn, lineNumber);
                tokenIndex = -1;
            }

            file.Close();            
            linesFromFile.Add(fn, l);
        }

        private void parseFromDirectory(string dirName,bool getExtension)
        {
            foreach (string file in Directory.GetFiles(dirName))
            {
                string ext = Path.GetExtension(file);
                if (getExtension)
                {
                    if (!allExtensions.Contains(ext)) allExtensions.Add(ext);
                }
                else
                {
                    if (included.Contains(ext))
                    {
                        parseFromFile(file);
                        countIncluded++;
                    }
                    countFiles++;
                }
            }
            foreach (string directory in Directory.GetDirectories(dirName))
            {
                parseFromDirectory(directory,getExtension);
            }
            if (!getExtension) countDirectories++;
        }

        private void getExtensions(string path)
        {
            allExtensions = new List<string>();
            included = new List<string>();
            parseFromDirectory(path, true);
            Form prompt = new Form();
            prompt.AutoSize = true;
            prompt.Text = "Choose Extensions";
            FlowLayoutPanel panel = new FlowLayoutPanel();
            panel.AutoSize = true;
            List<CheckBox> lc = new List<CheckBox>();
            for (int i = 0; i < allExtensions.Count; i++)
            {
                lc.Add(new CheckBox());
                lc[i].Text = allExtensions[i];
                panel.Controls.Add(lc[i]);
                panel.SetFlowBreak(lc[i], true);
            }
            Button ok = new Button() { Text = "Confirm" };
            ok.Click += (sender, e) => {
                for (int i = 0; i < lc.Count; i++)
                {
                    if (lc[i].Checked) included.Add(lc[i].Text);
                }
                prompt.Close(); 
            };
            Button no = new Button() { Text = "Cancel" };
            no.Click += (sender, e) => { prompt.Close(); };
            panel.Controls.Add(ok);
            panel.Controls.Add(no);
            prompt.Controls.Add(panel);            
            prompt.ShowDialog();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Console.WriteLine(sender.ToString()+e.ToString());
            FolderBrowserDialog browser = new FolderBrowserDialog();
            browser.ShowNewFolderButton = false;
            if ((comboBox1.Text != string.Empty) && Directory.Exists(comboBox1.Text))
                browser.SelectedPath = comboBox1.Text;
            if (browser.ShowDialog() == DialogResult.OK && Directory.Exists(browser.SelectedPath))
            {
                comboBox1.Text = browser.SelectedPath;
                getExtensions(browser.SelectedPath); 
                if (included.Count<=0) return;
                allTokens = new Dictionary<string, List<Tuple<string, int>>>();
                linesFromFile = new Dictionary<string, List<string>>();
                countDirectories = 0;
                countFiles = 0;
                countIncluded = 0;
                countTokens = 0;
                countLines = 0;
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Goldenrod;
                richTextBox1.AppendText("\r\nReading from " + browser.SelectedPath+ " ......\r\n");
                richTextBox1.Refresh();                
                timer = Stopwatch.StartNew();
                parseFromDirectory(browser.SelectedPath,false);
                timer.Stop();
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Goldenrod;
                richTextBox1.AppendText("Found " + countTokens + " tokens (" + allTokens.Keys.Count + " unique) in " + countLines + " lines.\r\n");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Goldenrod;
                richTextBox1.AppendText("Included: " + countIncluded + " Total: " + countFiles + " Dirs: " + countDirectories + "\r\n");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = Color.Goldenrod;
                richTextBox1.AppendText("Parsing completed (" + timer.ElapsedMilliseconds + "ms)\r\n");
                richTextBox1.Select(richTextBox1.TextLength, 0);
                richTextBox1.SelectionColor = richTextBox1.ForeColor;
                richTextBox1.AppendText(">> ");
                lastPoint = richTextBox1.TextLength;
                textBox1.Enabled = true;
            }
                
        }

        private void OnFormLoad(object sender, EventArgs e)
        {

        }

        private void textBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode==Keys.Return) {
                string s = textBox1.Text.Trim();
                history.Add(s);
                historyIndex = history.Count;
                if (comboBox2.Text == string.Empty) maxLines = 50;
                else maxLines = int.Parse(comboBox2.Text);
                findAllOccurences(s);
                richTextBox1.ScrollToCaret();
                lastPoint = richTextBox1.TextLength;
                textBox1.Clear();
            }
            else if (e.KeyCode == Keys.Up&&historyIndex>0)
            {
                historyIndex--;
                textBox1.Text = history[historyIndex];
            }
            else if (e.KeyCode == Keys.Down && historyIndex<history.Count-1)
            {
                historyIndex++;
                textBox1.Text = history[historyIndex];
            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            richTextBox1.ReadOnly = false;
            richTextBox1.Select(lastPoint, richTextBox1.TextLength - lastPoint);
            richTextBox1.Cut();
            richTextBox1.AppendText(textBox1.Text);
            richTextBox1.ReadOnly = true;
        }
    }
}
