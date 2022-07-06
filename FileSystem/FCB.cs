using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileSystem
{
    public enum FileType { folder, txt };

    [Serializable]
    public class FCB
    {
        public int fileId; // 通过共同的文件id来与FCB的符号目录项联系起来，一开始是想在符号目录项中增加一个指针，指向基本目录项，但是C#中指针只能在不安全的上下文中使用，因此无法使用指针，但是如果在FCBid类中增加一个FCBmain，便失去了将两者分开带来的减少磁盘IO优势。因此最终使用一个类
        public FileType type;

        public IndexTable indexTable = new IndexTable();

        public FCB parent = null;
        public FCB firstChild = null;
        public FCB nextBrother = null;

        public string fileName;
        public string filePath;
        public int fileSize; // 单位是Byte

        public DateTime creationTime;
        public DateTime lastModifiedTime;


        // 设置FCB内容
        public FCB(int fileId, FileType type, FCB parent, string name = "")
        {
            this.fileId = fileId;
            this.type = type;
            this.fileName = name;
            this.parent = parent;

            if (parent == null && fileId == 0)
            {
                fileName = "root";
                filePath = "root";
                return;
            }

            // 普通文件

            // 先设置FCB的树形目录
            if (parent != null && parent.firstChild == null)
            {
                parent.firstChild = this;
            }
            else if (parent != null)
            {
                FCB node = parent.firstChild;
                while (node.nextBrother != null)
                {
                    node = node.nextBrother;
                }
                // 所有文件按照创建的顺序在目录树中被存储
                node.nextBrother = this;
            }

            // 设置FCB的具体信息

            if (fileName == "")//使用默认名字
            {
                if (type == FileType.folder)
                {
                    fileName = "新建文件夹" + fileId.ToString();
                }
                else
                {
                    fileName = "新建文本文档" + fileId.ToString() + ".txt";
                }
            }

            if (fileId != 0)//不是根目录
            {
                filePath = parent.filePath + "/" + fileName;
            }
            creationTime = DateTime.Now;
            lastModifiedTime = DateTime.Now;

        }

        //由于递归的结构，必须有参数，否则无法传递条件
        public void GetAllChildsId(List<int> childs)
        {
            FCB child = firstChild;

            while (child != null)
            {
                childs.Add(child.fileId);
                if (child.type == FileType.folder)
                {
                    child.GetAllChildsId(childs);
                }
                child = child.nextBrother;
            }
            return;
        }



    }
}
