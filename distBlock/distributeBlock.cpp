#include <stdio.h>
#include <map>
#include <vector>
#include <iostream>

typedef struct ListContainer {
  std::string keyName;
  int64_t size;
} ListContainer;

typedef struct metaInfo
{
    bool exists;
    std::string fileName;
    int64_t fileLength;
    int64_t blockSize;
    int64_t rangeOffset;
    int64_t rangeOffsetEnd;
    int compress;

    metaInfo() {
        exists = false;
        fileName = "";
        fileLength = 0;
        blockSize = 0;
        rangeOffset = 0;
        rangeOffsetEnd = 0;
        compress = 0;
    }
} metaInfo;

class distributeBlockBase
{
public:

    void buildBlock(bool hiveTranscation, int segmentId, int segmentNum, int blockSize, std::vector<ListContainer> lists);

    void distributionBlock();

    int distributeBlockBegin;

    int distributeBlockEnd;

    std::map<int, metaInfo> block;

    int64_t mBlockSize;

    std::string curFileName;

    std::vector<ListContainer> normalLists;

    std::vector<ListContainer> deleteDeltaLists;

private:

    void filterList(bool hiveTranscation, std::vector<ListContainer> lists);

    void buildBigFile(int64_t &index, std::string &name, int64_t size);

    void updateOldBlock(int64_t index, std::string &name, int64_t size, int64_t blockSize, bool exists, int64_t rangeOffset, int64_t rangeLength);

    void insertNewBlock(int64_t index, std::string &name, int64_t size, int64_t blockSize, bool exists, int64_t rangeOffset, int64_t rangeLength);

    void buildSmallFile(int64_t index, std::string &name, int64_t size);

private:

    int segid;
    int segnum;

};


void distributeBlockBase::buildSmallFile(int64_t index, std::string &name, int64_t size)
{
    metaInfo info;
    info.blockSize = mBlockSize;
    info.exists = true;
    info.fileLength = size;
    info.fileName = name;
    info.rangeOffset = 0;
    info.rangeOffsetEnd = size;
    block.insert(std::make_pair(index, info));
}

void distributeBlockBase::insertNewBlock(int64_t index, std::string &name, int64_t size, int64_t blockSize, bool exists, int64_t rangeOffset, int64_t rangeOffsetEnd)
{
    metaInfo info;
    info.blockSize = blockSize;
    info.exists = exists;
    info.fileLength = size;
    info.fileName = name;
    info.rangeOffset = rangeOffset;
    info.rangeOffsetEnd = rangeOffsetEnd;
    block.insert(std::make_pair(index, info));
}

void distributeBlockBase::updateOldBlock(int64_t index, std::string &name, int64_t size, int64_t blockSize, bool exists, int64_t rangeOffset, int64_t rangeOffsetEnd)
{
    auto it = block.find(index);
    if (it != block.end())
    {
        auto oldInfo = it->second;
        metaInfo info;
        info.blockSize = oldInfo.blockSize;
        info.exists = oldInfo.exists;
        info.fileLength = oldInfo.fileLength;
        info.fileName = oldInfo.fileName;
        info.rangeOffset = oldInfo.rangeOffset;
        info.rangeOffsetEnd = rangeOffsetEnd;
        block[index] = info;
    }
    else
    {
    }
}

void distributeBlockBase::buildBigFile(int64_t &index, std::string &name, int64_t size)
{
    int64_t remainderSize = size % mBlockSize;
    if (remainderSize == 0)
    {
        for (int64_t offset = 0; offset < size; offset += mBlockSize)
        {
            insertNewBlock(index, name, size, mBlockSize, true, offset, offset + mBlockSize);
            index += 1;
        }
    }
    else
    {
        int64_t halfSize = mBlockSize / 2;
        int64_t offset = 0;
        int64_t remainderLength = size;
        for (; remainderLength > mBlockSize; remainderLength -= mBlockSize)
        {
            insertNewBlock(index, name, size, mBlockSize, true, offset, offset + mBlockSize);
            index += 1;
            offset += mBlockSize;
        }

        if (remainderLength > halfSize)
        {
            insertNewBlock(index, name, size, mBlockSize, true, offset, offset + remainderLength);
            index += 1;
        }
        else
        {
            index -= 1;
            updateOldBlock(index, name, size, mBlockSize, true, offset, offset + remainderLength);
            index += 1;
        }
    }
}

void distributeBlockBase::filterList(bool hiveTranscation, std::vector<ListContainer> lists)
{
    for (auto it = lists.begin(); it != lists.end(); it++)
    {
        ListContainer content;
        content.keyName = it->keyName;
        content.size = it->size;
        if (hiveTranscation)
        {
            deleteDeltaLists.push_back(content);
        }
        else
        {
            normalLists.push_back(content);
        }
    }
}

void distributeBlockBase::buildBlock(bool hiveTranscation, int segmentId, int segmentNum,  int blockSize, std::vector<ListContainer> lists)
{
    int64_t index = 0;
    mBlockSize = blockSize;
    segid = segmentId;
    segnum = segmentNum;
    filterList(hiveTranscation, lists);

    for (auto it = normalLists.begin(); it != normalLists.end(); it++)
    {
        if (it->size <= mBlockSize)
        {
            buildSmallFile(index, it->keyName, it->size);
            index += 1;
        }
        else
        {
            buildBigFile(index, it->keyName, it->size);
        }
    }

    int64_t remainderSize = block.size() % segnum;
    if (remainderSize != 0)
    {
        for (int64_t i = 0; i < segnum - remainderSize; i++)
        {
            std::string name = "";
            insertNewBlock(index, name, 0, mBlockSize, false, 0, 0);
            index += 1;
        }
    }
}

void distributeBlockBase::distributionBlock()
{
    if (segnum <= 0)
    {
    }
    int64_t num = block.size() / segnum;
    if (num < 0)
    {
        int64_t count = block.size();
    }
    // segid num begin is 0
    distributeBlockBegin = (segid) * (num);
    distributeBlockEnd = (segid + 1) * num - 1;
}

std::vector<ListContainer> append_testdata() {
    std::vector<ListContainer> lists;
    ListContainer a;
    a.keyName = "000000_0";
    a.size = 200588998;
    lists.push_back(a);

    a.keyName = "000002_0";
    a.size = 194202545;
    lists.push_back(a);

    a.keyName = "000004_0";
    a.size = 204497124;
    lists.push_back(a);

    a.keyName = "000005_0";
    a.size = 200901633;
    lists.push_back(a);

    a.keyName = "000006_0";
    a.size = 197580415;
    lists.push_back(a);

    a.keyName = "000010_0";
    a.size = 353561483;
    lists.push_back(a);

    a.keyName = "000011_0";
    a.size = 134109940;
    lists.push_back(a);

    a.keyName = "000012_0";
    a.size = 346021834;
    lists.push_back(a);

    a.keyName = "000013_0";
    a.size = 151023647;
    lists.push_back(a);

    a.keyName = "000014_0";
    a.size = 206585480;
    lists.push_back(a);

    a.keyName = "000015_0";
    a.size = 151315616;
    lists.push_back(a);

    a.keyName = "000016_0";
    a.size = 152582217;
    lists.push_back(a);

    a.keyName = "000017_0";
    a.size = 145984134;
    lists.push_back(a);

    a.keyName = "000023_0";
    a.size = 271746796;
    lists.push_back(a);

    a.keyName = "000024_0";
    a.size = 273878103;
    lists.push_back(a);

    a.keyName = "000027_0";
    a.size = 270044462;
    lists.push_back(a);

    a.keyName = "000030_0";
    a.size = 285318402;
    lists.push_back(a);

    a.keyName = "000037_0";
    a.size = 270991711;
    lists.push_back(a);

    a.keyName = "000038_0";
    a.size = 272336123;
    lists.push_back(a);

    a.keyName = "000041_0";
    a.size = 271931168;
    lists.push_back(a);

    a.keyName = "000047_0";
    a.size = 274218151;
    lists.push_back(a);

    a.keyName = "000054_0";
    a.size = 267936677;
    lists.push_back(a);

    a.keyName = "000059_0";
    a.size = 287513274;
    lists.push_back(a);

    a.keyName = "000061_0";
    a.size = 262005766;
    lists.push_back(a);

    a.keyName = "000063_0";
    a.size = 271384906;
    lists.push_back(a);

    a.keyName = "000070_0";
    a.size = 276210374;
    lists.push_back(a);

    a.keyName = "000072_0";
    a.size = 273495239;
    lists.push_back(a);

    a.keyName = "000075_0";
    a.size = 289178203;
    lists.push_back(a);

    a.keyName = "000076_0";
    a.size = 209854656;
    lists.push_back(a);

    a.keyName = "000079_0";
    a.size = 212675318;
    lists.push_back(a);

    a.keyName = "000083_0";
    a.size = 204712699;
    lists.push_back(a);

    a.keyName = "000087_0";
    a.size = 204442767;
    lists.push_back(a);

    a.keyName = "000089_0";
    a.size = 203349942;
    lists.push_back(a);

    a.keyName = "000091_0";
    a.size = 198803156;
    lists.push_back(a);

    a.keyName = "000094_0";
    a.size = 202651701;
    lists.push_back(a);

    a.keyName = "000095_0";
    a.size = 200893325;
    lists.push_back(a);

    a.keyName = "000098_0";
    a.size = 200925153;
    lists.push_back(a);

    a.keyName = "0000102_0";
    a.size = 197611392;
    lists.push_back(a);

    a.keyName = "0000104_0";
    a.size = 197134805;
    lists.push_back(a);

    a.keyName = "0000106_0";
    a.size = 162335174;
    lists.push_back(a);

    a.keyName = "0000108_0";
    a.size = 120579717;
    lists.push_back(a);

    a.keyName = "0000109_0";
    a.size = 119957613;
    lists.push_back(a);

    a.keyName = "0000110_0";
    a.size = 117624985;
    lists.push_back(a);

    a.keyName = "0000111_0";
    a.size = 143090673;
    lists.push_back(a);

    a.keyName = "0000112_0";
    a.size = 122082217;
    lists.push_back(a);

    a.keyName = "0000113_0";
    a.size = 120951919;
    lists.push_back(a);

    a.keyName = "0000114_0";
    a.size = 87872191;
    lists.push_back(a);

    return lists;
}

// std::vector<ListContainer> append_testdata() {
//     std::vector<ListContainer> lists;
//     ListContainer a;
//     a.keyName = "test";
//     a.size = 6352153;
//     lists.push_back(a);
//     return lists;
// }

void forloop(int segid, int segnum) {
    std::vector<ListContainer> list = append_testdata();
    distributeBlockBase mdist;
    mdist.buildBlock(false, segid, segnum, 16*1024*1024, list);
    mdist.distributionBlock();

    for (auto iter = mdist.block.begin(); iter != mdist.block.end(); iter++) {
        printf("segid:%d %d %s %ld %ld %ld %ld\n", segid, iter->first, 
            iter->second.fileName.c_str(),
            iter->second.fileLength,
            iter->second.blockSize,
            iter->second.rangeOffset,
            iter->second.rangeOffsetEnd);
    }
    printf("b %d e %d\n", mdist.distributeBlockBegin, mdist.distributeBlockEnd);

}

//g++ distributeBlock.cpp -o distributeBlock -g -O0 -std=c++11
int main(int argc, char* argv[]) {
    for (int i = 0; i < 32; i++) {
        forloop(i, 32);
        break;
    }
    return 0;
}