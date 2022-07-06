using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FileSystem
{
    [Serializable]
    public class DiskBlock
    {
        private char[] data;

        public DiskBlock()
        {
            data = new char[8]; // a block can contain 8 char character
            Clear();
        }

        public void Clear()
        {
            for (int i = 0; i < 8; i++)
            {
                data[i] = '\0';
            }
        }

        public void SetBlock(string data)
        {
            Clear();
            for (int i = 0; i < data.Length && i < 8; i++)
            {
                this.data[i] = data[i];
            }
        }

        public string GetBlockData()
        {
            string res = new(data);
            return res;
        }
    }
}
