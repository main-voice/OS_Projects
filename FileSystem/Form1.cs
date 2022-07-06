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
using System.Runtime.Serialization.Formatters.Binary;
using Microsoft.VisualBasic;

namespace FileSystem
{
    public partial class Form1 : Form
    {
        const string historyData = "history data";
        public FileManager fileManager = new FileManager();

        public int currentFolderID = 0; //当前目录文件夹第一个指针
        private Dictionary<ListViewItem, int> currentFolderTable = new Dictionary<ListViewItem, int>();//将ListView中每一项都与一个fileID对应,listTable可以由currntFolderID得到
        public Form1()
        {
            InitializeComponent();


            listView1.View = View.Details; // 表格在窗体显示细节的格式，如果要做成表格，那么视图必须要选择Details
            listView1.Columns.Add("文件名", 300);
            listView1.Columns.Add("文件大小", 150);
            listView1.Columns.Add("创建时间", 350);
            listView1.Columns.Add("修改时间", 350);

            //foreach(ColumnHeader columnHeader in listView1.Columns)
            //{
            //    columnHeader.Width = -1;
            //}
            // listView1.FullRowSelect = true;

            // 必须要先创建列才可以往里面加item，否则无法显示

            fileManager.CreateFile(FileType.folder);//创建根目录

            if (MessageBox.Show("是否加载历史数据?", "提示", MessageBoxButtons.YesNo) == DialogResult.Yes)
                LoadHistoryData();

            updateListView();
            updateTreeView();
            updateCurrentPath();
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void 新建文件夹ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            fileManager.currentFolderId = this.currentFolderID;
            fileManager.CreateFile(FileType.folder);
            updateListView();
            updateTreeView();
        }

        private void 新建文件ToolStripMenuItem_Click(object sender, EventArgs e)
        {

            fileManager.currentFolderId = this.currentFolderID;
            fileManager.CreateFile(FileType.txt);
            updateListView();
            updateTreeView();
        }

        private void updateListView() // 更新文件视图
        {
            listView1.Items.Clear();

            // current folder
            FCB curFolder = fileManager.FCBTable[currentFolderID];
            

            FCB curFile = curFolder.firstChild;

            while (curFile != null)
            {
                int fileId = curFile.fileId;

                int size = fileManager.GetSize(fileId);
                string strSize = "";
                if (size / 1024 > 0)
                {
                    strSize = (size / 1024).ToString() + "KB";
                }
                else
                {
                    strSize = (size).ToString() + "B";
                }

                string[] info = {
                    curFile.fileName,
                    strSize,
                    curFile.creationTime.ToString(),
                    curFile.lastModifiedTime.ToString()
                };

                ListViewItem item = new ListViewItem(info);
                if (curFile.type == FileType.folder)
                    item.ImageIndex = 0;
                else
                    item.ImageIndex = 1;

                listView1.Items.Add(item);

                currentFolderTable.Add(item, curFile.fileId);

                curFile = curFile.nextBrother;

                //listView1.Columns[列的索引].Width = -1;
            }
            
        }

        private void updateTreeView()
        {
            treeView1.Nodes.Clear();
            TreeNode rootNode = new TreeNode("root");
            updateTreeNode(rootNode, 0);
            treeView1.Nodes.Add(rootNode);
            treeView1.ExpandAll();

        }

        private void updateTreeNode(TreeNode treeNode, int curFileId)
        {
            FCB currentFCB = fileManager.FCBTable[curFileId];
            FCB child = currentFCB.firstChild;

            while (child != null)
            {
                TreeNode sonNode = new TreeNode(child.fileName);

                sonNode.ImageIndex = 1;

                //update the whole folder recursively
                if (child.type == FileType.folder)
                {
                    sonNode.ImageIndex = 0;
                    updateTreeNode(sonNode, child.fileId);
                }

                treeNode.Nodes.Add(sonNode);
                child = child.nextBrother;
            }
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        // enter a folder or open a file
        private void listView1_DoubleClick(object sender, EventArgs e)
        {
            // Console.WriteLine("double click file");
            if (listView1.SelectedItems.Count == 0)
            {
                return;
            }

            ListViewItem curFileItem = new ListViewItem();
            curFileItem = listView1.SelectedItems[0];

            // get the id of the file
            int curFileId = currentFolderTable[curFileItem];
            FCB fCB = fileManager.FCBTable[curFileId];

            // enter folder
            if (fCB.type == FileType.folder)
            {
                currentFolderID = curFileId;
                updateListView();
                updateCurrentPath();
            }
            else // open a txt
            {

                //Console.WriteLine("文件原本内容为：");

                //string oldData = fileManager.GetFileData(curFileId);
                //Console.WriteLine(oldData);

                //Console.WriteLine("请输入文本的内容");
                //string data = Console.ReadLine();

                // string str1 = Interaction.InputBox("保存的数据名", "保存", "文本内容", -1, -1);

                string oldData = fileManager.GetFileData(curFileId);
                Console.WriteLine(oldData);


                string data = Interaction.InputBox("文本", "编辑文本", oldData, -1, -1);

                if (MessageBox.Show("是否确定修改文件内容?", "提示", MessageBoxButtons.YesNo) == DialogResult.No)
                {
                    fileManager.EditFile(curFileId, oldData);
                    fileManager.FCBTable[curFileId].fileSize = data.Count() * 2;
                    return;
                }

                fileManager.EditFile(curFileId, data);
                fileManager.FCBTable[curFileId].fileSize = data.Count() * 2;

                fileManager.FCBTable[curFileId].lastModifiedTime = DateTime.Now;
                updateListView();

            };

        }



        private void button1_Click(object sender, EventArgs e)
        {
            if (currentFolderID != 0)//不为根文件夹才能返回
                currentFolderID = fileManager.FCBTable[currentFolderID].parent.fileId;
            updateListView();
            updateCurrentPath();
        }

        private void updateCurrentPath()//更新当前路径显示
        {
            textBox1.Clear();
            textBox1.Text = fileManager.FCBTable[currentFolderID].filePath;
        }

        private void 格式化ToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            if (MessageBox.Show("是否清除当前文件系统中所有数据?", "提示", MessageBoxButtons.YesNo) == DialogResult.No)
                return;

            this.fileManager = new FileManager();
            currentFolderTable = new Dictionary<ListViewItem, int>();
            currentFolderID = 0;
            fileManager.CreateFile(FileType.folder);
            updateListView();
            updateTreeView();
            updateCurrentPath();
        }


        public void SaveAllData() // 保存fileManager中数据
        {
            string path = Directory.GetCurrentDirectory();
            // string path = "D:\\test\\test1\\demo";
            //Directory.SetCurrentDirectory(Directory.GetParent(path).FullName);
            //string realPath = Directory.GetCurrentDirectory();

            //Console.WriteLine("save history:");
            //Console.WriteLine(realPath + "\\" + historyData);

            FileStream f = new FileStream(System.IO.Path.Combine(path, historyData), FileMode.Create);
            BinaryFormatter b = new BinaryFormatter();
            b.Serialize(f, fileManager);
            MessageBox.Show("历史数据已保存至" + System.IO.Path.Combine(path, historyData), "提示");
            f.Close();
        }

        public void LoadHistoryData()//加载历史数据
        {
            string path = Directory.GetCurrentDirectory();
            // string path = "D:\\test\\test1\\demo";
            //Directory.SetCurrentDirectory(Directory.GetParent(path).FullName);
            //string realPath = Directory.GetCurrentDirectory();

            if (!File.Exists(System.IO.Path.Combine(path, historyData)))
            {
                MessageBox.Show("找不到历史数据文件" + System.IO.Path.Combine(path
, historyData) + " \n将新建文件系统!", "提示");
                return;
            }
            //Console.WriteLine("find history:");
            //Console.WriteLine(path + "\\" + historyData);

            FileStream f = new FileStream(System.IO.Path.Combine(path, historyData), FileMode.Open, FileAccess.Read, FileShare.Read);
            BinaryFormatter b = new BinaryFormatter();
            fileManager = (FileManager)b.Deserialize(f);
            f.Close();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (MessageBox.Show("是否保存当前文件系统中数据?", "提示", MessageBoxButtons.YesNo) == DialogResult.Yes)
                SaveAllData();
        }

        private void 重命名ToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            // 在一个listview中所有的item，如果选中，都会被放在SelectedItems中，当只选中一个时，也就是索引为0
            // 然后通过这个item得到对应的fileid，进而得到FCB，在此listView对于文件操作都是这个思路
            if (listView1.SelectedItems.Count == 0)
            {
                return;
            }
            ListViewItem currentItem = new ListViewItem();
            currentItem = listView1.SelectedItems[0];//当前所选项

            int currentFileID = currentFolderTable[currentItem];//得到当前所选项对应fileID
            FCB currentFCB = fileManager.FCBTable[currentFileID];

            //Console.WriteLine("请输入修改后的名字");
            //string newName = Console.ReadLine();

            string newName = Microsoft.VisualBasic.Interaction.InputBox("请输入修改后的名字", "保存", "文本内容", -1, -1);
            if (currentFCB.type == FileType.txt)
            {
                newName += ".txt";
            }
            fileManager.RenameFile(currentFileID, newName);
            updateListView();
            updateTreeView();

        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count == 0)
            {
                return;
            }
            ListViewItem currentItem = new ListViewItem();

            // 在一个listview中所有的item，如果选中，都会被放在SelectedItems中，当只选中一个时，也就是索引为0
            currentItem = listView1.SelectedItems[0];

            // 然后通过这个item得到对应的fileid，进而得到FCB，在此listView对于文件操作都是这个思路
            int currentFileID = currentFolderTable[currentItem];//得到当前所选项对应fileID

            fileManager.DeleteFile(currentFileID);

            updateListView();
            updateTreeView();

        }

        private void 保存ToolStripMenuItem_Click(object sender, EventArgs e)
        {

            string path = Directory.GetCurrentDirectory();
            // Console.WriteLine("path: " + path);

            //string parentPath = Directory.GetParent(path).FullName;
            //Console.WriteLine("parentPath: " + parentPath);

            //string realPath = Directory.GetCurrentDirectory();
            //Console.WriteLine("realPath: " + realPath);

            //Console.WriteLine("save history:");
            //Console.WriteLine(realPath + "\\" + historyData);

            FileStream f = new FileStream(System.IO.Path.Combine(path, historyData), FileMode.Create);
            BinaryFormatter b = new BinaryFormatter();
            b.Serialize(f, fileManager);
            MessageBox.Show("历史数据已保存至" + System.IO.Path.Combine(path, historyData), "提示");
            f.Close();
        }
    }
};