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

        public int currentFolderID = 0; //��ǰĿ¼�ļ��е�һ��ָ��
        private Dictionary<ListViewItem, int> currentFolderTable = new Dictionary<ListViewItem, int>();//��ListView��ÿһ���һ��fileID��Ӧ,listTable������currntFolderID�õ�
        public Form1()
        {
            InitializeComponent();


            listView1.View = View.Details; // ����ڴ�����ʾϸ�ڵĸ�ʽ�����Ҫ���ɱ����ô��ͼ����Ҫѡ��Details
            listView1.Columns.Add("�ļ���", 300);
            listView1.Columns.Add("�ļ���С", 150);
            listView1.Columns.Add("����ʱ��", 350);
            listView1.Columns.Add("�޸�ʱ��", 350);

            //foreach(ColumnHeader columnHeader in listView1.Columns)
            //{
            //    columnHeader.Width = -1;
            //}
            // listView1.FullRowSelect = true;

            // ����Ҫ�ȴ����вſ����������item�������޷���ʾ

            fileManager.CreateFile(FileType.folder);//������Ŀ¼

            if (MessageBox.Show("�Ƿ������ʷ����?", "��ʾ", MessageBoxButtons.YesNo) == DialogResult.Yes)
                LoadHistoryData();

            updateListView();
            updateTreeView();
            updateCurrentPath();
        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void �½��ļ���ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            fileManager.currentFolderId = this.currentFolderID;
            fileManager.CreateFile(FileType.folder);
            updateListView();
            updateTreeView();
        }

        private void �½��ļ�ToolStripMenuItem_Click(object sender, EventArgs e)
        {

            fileManager.currentFolderId = this.currentFolderID;
            fileManager.CreateFile(FileType.txt);
            updateListView();
            updateTreeView();
        }

        private void updateListView() // �����ļ���ͼ
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

                //listView1.Columns[�е�����].Width = -1;
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

                //Console.WriteLine("�ļ�ԭ������Ϊ��");

                //string oldData = fileManager.GetFileData(curFileId);
                //Console.WriteLine(oldData);

                //Console.WriteLine("�������ı�������");
                //string data = Console.ReadLine();

                // string str1 = Interaction.InputBox("�����������", "����", "�ı�����", -1, -1);

                string oldData = fileManager.GetFileData(curFileId);
                Console.WriteLine(oldData);


                string data = Interaction.InputBox("�ı�", "�༭�ı�", oldData, -1, -1);

                if (MessageBox.Show("�Ƿ�ȷ���޸��ļ�����?", "��ʾ", MessageBoxButtons.YesNo) == DialogResult.No)
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
            if (currentFolderID != 0)//��Ϊ���ļ��в��ܷ���
                currentFolderID = fileManager.FCBTable[currentFolderID].parent.fileId;
            updateListView();
            updateCurrentPath();
        }

        private void updateCurrentPath()//���µ�ǰ·����ʾ
        {
            textBox1.Clear();
            textBox1.Text = fileManager.FCBTable[currentFolderID].filePath;
        }

        private void ��ʽ��ToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            if (MessageBox.Show("�Ƿ������ǰ�ļ�ϵͳ����������?", "��ʾ", MessageBoxButtons.YesNo) == DialogResult.No)
                return;

            this.fileManager = new FileManager();
            currentFolderTable = new Dictionary<ListViewItem, int>();
            currentFolderID = 0;
            fileManager.CreateFile(FileType.folder);
            updateListView();
            updateTreeView();
            updateCurrentPath();
        }


        public void SaveAllData() // ����fileManager������
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
            MessageBox.Show("��ʷ�����ѱ�����" + System.IO.Path.Combine(path, historyData), "��ʾ");
            f.Close();
        }

        public void LoadHistoryData()//������ʷ����
        {
            string path = Directory.GetCurrentDirectory();
            // string path = "D:\\test\\test1\\demo";
            //Directory.SetCurrentDirectory(Directory.GetParent(path).FullName);
            //string realPath = Directory.GetCurrentDirectory();

            if (!File.Exists(System.IO.Path.Combine(path, historyData)))
            {
                MessageBox.Show("�Ҳ�����ʷ�����ļ�" + System.IO.Path.Combine(path
, historyData) + " \n���½��ļ�ϵͳ!", "��ʾ");
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
            if (MessageBox.Show("�Ƿ񱣴浱ǰ�ļ�ϵͳ������?", "��ʾ", MessageBoxButtons.YesNo) == DialogResult.Yes)
                SaveAllData();
        }

        private void ������ToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            // ��һ��listview�����е�item�����ѡ�У����ᱻ����SelectedItems�У���ֻѡ��һ��ʱ��Ҳ��������Ϊ0
            // Ȼ��ͨ�����item�õ���Ӧ��fileid�������õ�FCB���ڴ�listView�����ļ������������˼·
            if (listView1.SelectedItems.Count == 0)
            {
                return;
            }
            ListViewItem currentItem = new ListViewItem();
            currentItem = listView1.SelectedItems[0];//��ǰ��ѡ��

            int currentFileID = currentFolderTable[currentItem];//�õ���ǰ��ѡ���ӦfileID
            FCB currentFCB = fileManager.FCBTable[currentFileID];

            //Console.WriteLine("�������޸ĺ������");
            //string newName = Console.ReadLine();

            string newName = Microsoft.VisualBasic.Interaction.InputBox("�������޸ĺ������", "����", "�ı�����", -1, -1);
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

            // ��һ��listview�����е�item�����ѡ�У����ᱻ����SelectedItems�У���ֻѡ��һ��ʱ��Ҳ��������Ϊ0
            currentItem = listView1.SelectedItems[0];

            // Ȼ��ͨ�����item�õ���Ӧ��fileid�������õ�FCB���ڴ�listView�����ļ������������˼·
            int currentFileID = currentFolderTable[currentItem];//�õ���ǰ��ѡ���ӦfileID

            fileManager.DeleteFile(currentFileID);

            updateListView();
            updateTreeView();

        }

        private void ����ToolStripMenuItem_Click(object sender, EventArgs e)
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
            MessageBox.Show("��ʷ�����ѱ�����" + System.IO.Path.Combine(path, historyData), "��ʾ");
            f.Close();
        }
    }
};