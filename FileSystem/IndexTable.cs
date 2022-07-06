using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// 文件信息存放在若干不连续物理块中，系统为每个文件建立一个专用数据结构--索引表，并将这些块的块号存放在一个索引表中；
//  一个索引表就是磁盘块地址数组,其中第i个条目指向文件的第i块。

namespace FileSystem
{
    // 文件在物理磁盘中采取索引存储的方式，并且采用综合模式索引，每个文件索引表为64个索引项，每项4个字节。前面32项直接登记存放文件信息的物理块号,如果文件大于32块，则利用第33-48项指向一个物理块，该块中最多可放64个文件物理块的块号。对于更大的文件还可利用49-64作为二次寻址；指向一个物理块，该块中最多可放64个文件物理块的块号
    // 一个文件一个索引表，表示在磁盘中如何存储
    [Serializable]
    public class IndexTable
    {
        public int directIndexNum;
        public int primaryIndexNum_1;
        public int primaryIndexNum_2;
        public int secondaryIndexNum_1;
        public int secondaryIndexNum_2;
        public int secondaryIndexNum_3;

        public int[] directIndex;       // 直接索引
        public int[][] indirectIndexOne;   // 一级间接索引
        public int[][][] indirectIndexTwo;//二级间接索引

        public IndexTable()
        {
            directIndex = new int[64];
            directIndexNum = -1;
        }

        public bool addIndex(int blockId)
        {
            if (indirectIndexOne == null)
            {
                directIndexNum++;
                directIndex[directIndexNum] = blockId;
                if (directIndexNum == 63) // 此时需要建立一级间接索引
                {
                    indirectIndexOne = new int[64][];
                    indirectIndexOne[0] = new int[64];
                    primaryIndexNum_1 = 0;
                    primaryIndexNum_2 = -1;
                }
            }
            else if (indirectIndexTwo == null)
            {
                primaryIndexNum_2++;
                indirectIndexOne[primaryIndexNum_1][primaryIndexNum_2] = blockId;
                if (primaryIndexNum_2 == 63)
                {
                    if (primaryIndexNum_1 != 63)
                    {
                        primaryIndexNum_1++;
                        indirectIndexOne[primaryIndexNum_1] = new int[64];
                        primaryIndexNum_2 = -1;
                    }
                    else
                    {
                        //此时需要建立三级索引
                        indirectIndexTwo = new int[64][][];
                        indirectIndexTwo[0] = new int[64][];
                        indirectIndexTwo[0][0] = new int[64];
                        secondaryIndexNum_1 = 0;
                        secondaryIndexNum_2 = 0;
                        secondaryIndexNum_3 = -1;
                    }
                }
            }
            else
            {
                secondaryIndexNum_3++;
                indirectIndexTwo[secondaryIndexNum_1][secondaryIndexNum_2][secondaryIndexNum_3] = blockId;
                if (secondaryIndexNum_3 == 63)
                {
                    if (secondaryIndexNum_2 != 63)
                    {
                        secondaryIndexNum_2++;
                        secondaryIndexNum_3 = -1;
                        indirectIndexTwo[secondaryIndexNum_1][secondaryIndexNum_2] = new int[63];
                    }
                    else if (secondaryIndexNum_1 != 63)
                    {
                        secondaryIndexNum_1++;
                        secondaryIndexNum_2 = 0;
                        secondaryIndexNum_3 = -1;

                        indirectIndexTwo[secondaryIndexNum_1] = new int[64][];
                        indirectIndexTwo[secondaryIndexNum_1][secondaryIndexNum_2] = new int[64];
                    }
                    else
                    {
                        MessageBox.Show("文件过大", "警告");
                        return false;
                    }
                }
            }
            return true;
        }

        public void addIndex(List<int> blockIDs)
        {
            foreach (int blockID in blockIDs)
            {
                addIndex(blockID);
            }
        }

        public List<int> getAllBlockIds()
        {
            if (directIndex == null) // 不占用空间
            {
                return new List<int>();
            }
            List<int> res = new();

            for (int i = 0; i <= directIndexNum; i++)
            {
                res.Add(directIndex[i]);
            }

            //一级间接索引
            if (indirectIndexOne != null)
            {
                // 用完的部分
                for (int i = 0; i < primaryIndexNum_1; i++)
                {
                    for (int j = 0; j <= 63; j++)
                    {
                        res.Add(indirectIndexOne[i][j]);
                    }
                }
                //多出来的部分，没有占完一个块
                for (int k = 0; k <= primaryIndexNum_2; k++)
                {
                    res.Add(indirectIndexOne[primaryIndexNum_1][k]);
                }
            }

            if (indirectIndexTwo != null)//二级间接索引
            {
                for (int i = 0; i < secondaryIndexNum_1; i++)
                {
                    for (int j = 0; j < 63; j++)
                    {
                        for (int k = 0; k < 63; k++)
                        {
                            res.Add(indirectIndexTwo[i][j][k]);
                        }
                    }
                }
                for (int i = 0; i < secondaryIndexNum_2; i++)
                {
                    for (int j = 0; j < 63; j++)
                    {
                        res.Add(indirectIndexTwo[secondaryIndexNum_1][i][j]);
                    }
                }
                for (int i = 0; i <= secondaryIndexNum_3; i++)
                {
                    res.Add(indirectIndexTwo[secondaryIndexNum_1][secondaryIndexNum_2][i]);
                }
            }
            return res;
        }

    }
}
