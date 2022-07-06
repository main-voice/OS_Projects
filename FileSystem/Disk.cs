using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileSystem
{
    [Serializable]
    public class Disk
    {
        private const int capacity = 1024 * 1024;

        public int DiskSize { get; set; }

        // current position
        public int curPosition = 0;

        public bool[] bitMap; // manage the free space
        public DiskBlock[] blocks; // blocks in the disk

        // methods
        public Disk()
        {
            bitMap = new bool[capacity];
            blocks = new DiskBlock[capacity];
        }

        //allocate disk block for data at the blockid-th block
        public void AllocBlock(string data, int blockId)
        {
            blocks[blockId] = new DiskBlock();
            blocks[blockId].SetBlock(data);
            bitMap[blockId] = true;
        }

        public void FreeBlock(List<int> blockId)
        {
            for (int i = 0; i < blockId.Count; i++)
            {
                // modify the bit map
                bitMap[blockId[i]] = false;
            }
        }

        
        public List<int> StoreData(string storeString)
        {
            List<int> result = new();
            while (storeString.Length > 8)
            {
                // current position is empty
                if (bitMap[curPosition] == false) {
                    string data = storeString.Substring(0, 7);
                    AllocBlock(data, curPosition);

                    result.Add(curPosition);
                    curPosition = (curPosition + 1) % capacity;
                    storeString = storeString.Remove(0, 7);
                }
                else {
                    int oldPosition = curPosition;
                    while (true)
                    {
                        if (bitMap[curPosition] == true)
                        {
                            curPosition = (curPosition + 1) % capacity;
                            // go back the original position
                            if(curPosition == oldPosition)
                            {
                                MessageBox.Show("exceed space limits!");
                                System.Environment.Exit(-1);
                            }
                        }

                        // find free block space
                        string data = storeString.Substring(0, 7);
                        AllocBlock(data, curPosition);

                        result.Add(curPosition);
                        curPosition = (curPosition + 1) % capacity;
                        storeString = storeString.Remove(0, 7);
                    }
                }
            }

            AllocBlock(storeString, curPosition);

            result.Add(curPosition);
            curPosition = (curPosition + 1) % capacity;

            return result;
        }

    }
}
