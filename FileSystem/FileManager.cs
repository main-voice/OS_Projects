using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileSystem
{
    [Serializable]
    public class FileManager
    {

        public Dictionary<int, FCB> FCBTable;
        public Disk disk;
        public int fileCount = 0;
        public int currentFolderId = 0 ;

        public FileManager()
        {
            FCBTable = new Dictionary<int, FCB>();
            disk = new Disk();
        }

        // create a new file 
        public int CreateFile(FileType fileType)
        {
            if (fileCount == 0)
            {
                FCB fcb = new FCB(fileCount, fileType, null, "root");
                FCBTable.Add(fcb.fileId, fcb);
            }
            else
            {
                FCB fcb = new FCB(fileCount, fileType, FCBTable[currentFolderId]);
                FCBTable.Add(fcb.fileId, fcb);
            }
            fileCount++;
            return fileCount - 1;
        }

        public bool DeleteFile(int targetFileId)
        {
            FCB targetFCB = FCBTable[targetFileId];

            FCB parentFCB = targetFCB.parent;

            if(parentFCB.firstChild == targetFCB)
            {
                parentFCB.firstChild = targetFCB.nextBrother;
            }
            else
            {
                FCB preBrother = parentFCB.firstChild;
                while (preBrother.nextBrother != targetFCB)
                {
                    preBrother = preBrother.nextBrother;
                }
                preBrother.nextBrother = targetFCB.nextBrother;
            }

            List<int> toDeleteFileIds = new List<int>(); // delete blocks the file took place
            // if the fcb is a folder, we need to delete its children recursively
            FCBTable[targetFileId].GetAllChildsId(toDeleteFileIds);
            toDeleteFileIds.Add(targetFileId);

            List<int> toFreeBlocks = new List<int>();
            foreach(int fileId in toDeleteFileIds)
            {
                // the index structure of current file
                IndexTable index = FCBTable[fileId].indexTable;
                if (index != null) // not empty
                {
                    List<int> blocks = index.getAllBlockIds();
                    foreach(int blockId in blocks)
                    {
                        toFreeBlocks.Add(blockId);
                    }
                }
                FCBTable.Remove(fileId);
            }

            disk.FreeBlock(toFreeBlocks);

            return true;
        }
        // edit a file
        public bool EditFile(int fileId, string newData)
        {
            // remove all before data, and insert the new data
            List<int> preBlocks = FCBTable[fileId].indexTable.getAllBlockIds();

            disk.FreeBlock(preBlocks);

            FCBTable[fileId].indexTable = new IndexTable();

            List<int> newBlocks = disk.StoreData(newData);

            FCBTable[fileId].indexTable.addIndex(newBlocks);

            return true;
        }

        public string GetFileData(int fileId)
        {
            string res = new("");
            List<int> blocks = FCBTable[fileId].indexTable.getAllBlockIds();

            foreach(var blockId in blocks)
            {
                res += disk.blocks[blockId].GetBlockData();
            }
            return res; 
        }

        public int GetSize(int fileId)
        {
            if (FCBTable[fileId].type == FileType.txt)
            {
                return FCBTable[fileId].fileSize;
            }
            // folder, get all txt size recursively
            int allSize = 0;
            FCB child = FCBTable[fileId].firstChild;
            while(child != null)
            {
                allSize += GetSize(child.fileId);
                child = child.nextBrother;
            }

            FCBTable[fileId].fileSize = allSize;
            return allSize;
        }

        public int GetDiskSize(int fileId)
        {
            decimal diskSize = 0;

            diskSize = Convert.ToDecimal(FCBTable[fileId].fileSize);
            diskSize = Math.Ceiling(diskSize / 16) * 16;
            return Convert.ToInt32(diskSize);
        }

        public bool RenameFile(int fileId, string name)
        {
            // avoid the collinsion between user-define name and default name "新建文件夹"
            if(FCBTable[fileId].type == FileType.txt)
            {
                if(name.Length >= 6 && name.Substring(0, 6) == "新建文本文档")
                {
                    MessageBox.Show("请不要以新建文件夹或者新建文本文档开头");
                    return false;
                }
            }
            else if(FCBTable[fileId].type == FileType.folder)
            {
                if(name.Length >= 5 && name.Substring(0,5) == "新建文件夹")
                {
                    MessageBox.Show("请不要以新建文件夹或者新建文本文档开头");
                    return false;
                }
            }

            FCB brother = FCBTable[fileId].parent.firstChild;
            while(brother != null)
            { 
                // same type, and ignore itself
                if(brother.type == FCBTable[fileId].type && brother.fileId != fileId)
                {
                    if(brother.fileName == name)
                    {
                        MessageBox.Show("同目录下存在相同名字的" + brother.type.ToString() + " , 重命名失败");
                        return false;
                    }
                }
                brother = brother.nextBrother;
            }

            FCBTable[fileId].fileName = name;
            
            return true;
        }
    }
}
