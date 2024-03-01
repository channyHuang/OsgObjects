#include "parseBag.h"

#include <opencv2/highgui/highgui.hpp>

ParseBag* ParseBag::instance = nullptr;

ParseBag::ParseBag() {}

ParseBag::~ParseBag() {}

void ParseBag::parseBag(const std::string& sFileName) {
    std::ifstream ifs(sFileName, std::ios::binary | std::ios::in);
    std::string line;
    std::getline(ifs, line);
    std::cout << line << std::endl;
    counts.resize(256, 0);

    // bag header record 
    readHeader(ifs, true);
    
    int opCount[8] = { 0 };
    int conn[10] = { 0 };
    // for each record
    while (!ifs.eof()) {
        // header
        int res = readHeader(ifs, false);
        if (res < 0) {
            std::cout << "read header len <= 0" << std::endl;
            for (int i = 0; i < 256; ++i) {
                std::cout << counts[i] << " ";
            }
            std::cout << std::endl;
            break;
        }
        opCount[std::stoi(mapHeaderAttribute["op"])]++;
        // data
        switch (std::stoi(mapHeaderAttribute["op"])) {
        case 2:
        {
            conn[std::stoi(mapHeaderAttribute["conn"])]++;
            switch (std::stoi(mapHeaderAttribute["conn"])) {
            case 0: // IMU
                readIMU(ifs);
                break;
            case 1: // Image
                readImage(ifs);
                break;
            case 2: // PointCloud
                readPoint(ifs);
                break;
            default:
                readData(ifs);
                break;
            }
        }
            break;
        case 5:
        {
            memset(clen, 0, 4);
            ifs.read((char*)&clen, sizeof(char) * 4);
            int datalen = toInt(clen, 0, 4);

            int curlen = 0;
            while (curlen < datalen) {
                curlen += readHeader(ifs);
                opCount[std::stoi(mapHeaderAttribute["op"])]++;

                if (std::stoi(mapHeaderAttribute["op"]) == 7) {
                    curlen += readHeader(ifs);
                }
                else if (std::stoi(mapHeaderAttribute["op"]) == 2) {
                    conn[std::stoi(mapHeaderAttribute["conn"])]++;
                    switch (std::stoi(mapHeaderAttribute["conn"])) {
                    case 0: // IMU
                        curlen += readIMU(ifs);
                        break;
                    case 1: // Image
                        curlen += readImage(ifs);
                        break;
                    case 2: // PointCloud
                        curlen += readPoint(ifs);
                        break;
                    default:
                        curlen += readData(ifs);
                        break;
                    }
                }
                else {
                    curlen += readData(ifs);
                }
            }
        }
        break;
        case 7:
        {
            readHeader(ifs);
        }
        break;
        default:
            readData(ifs);
            break;
        }
    }
    ifs.close();
    
    {
        std::cout << "opCount: " << std::endl;
        for (int i = 0; i < 8; ++i) {
            std::cout << opCount[i] << " ";
        }
        std::cout << " , conn = ";
        for (int i = 0; i < 10; ++i) {
            std::cout << conn[i] << " ";
        }
        std::cout << std::endl;
    }
}

int ParseBag::toInt(unsigned char* buf, int st, int len) {
    int res = 0;
    int base = 1;
    for (int i = 0; i < len; ++i) {
        //if (buf[i + st] == '0x20') break;
        //if (buf[i + st] == 0) break;
        res = res + buf[i + st] * base;
        base *= 256;
    }
    return res;
}

int ParseBag::toString(unsigned char* buf, int st, int len, std::string& name, std::string& value) {
    char cname[4096] = { 0 }, cval[4096] = { 0 };
    bool isval = false, isnum = true;
    int base = 1;
    int val = 0;
    int i = 0;
    int idx = 0;
    for (i = 0; i < len; ++i) {
        if (buf[st + i] == '=') {
            break;
        }
        cname[i] = buf[st + i];
        if (!((buf[st + i] >= 'a' && buf[st + i] <= 'z') || (buf[st + i] >= 'A' && buf[st + i] <= 'Z')
            || (buf[st + i] == '_')
            || (buf[st + i] >= '0' && buf[st + i] <= '9'))) {
            std::cout << "error " << std::endl;
        }
    }
    name = std::string(cname);
    if (setHeaderString.find(name) != setHeaderString.end()) {
        isnum = false;
        val = -1;
    }

    for (++i; i < len; ++i) {
        if (isnum) {
            if (buf[st + i] == 0 || buf[st + i] == '0x20') break;
            val = val + buf[st + i] * base;
            base *= 256;
        }
        else {
            cval[idx++] = buf[st + i];
        }
    }

    value = std::string(cval);
    return val;
}

int ParseBag::readIMU(std::ifstream& ifs) {
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int datalen = toInt(clen, 0, 4);

    // header
    ifs.read((char*)&buffer, sizeof(char) * 4); // seq
    ifs.read((char*)&buffer, sizeof(char) * 8); // time

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // frame_id len
    int frameIdLen = toInt(clen, 0, 4);
    memset(buffer, 0, MAX_LEN);
    ifs.read((char*)&buffer, sizeof(char) * frameIdLen); // frame_id

    // data: orientation (quat), orientationCov (Matrix3x3), angularVel (Vec3), angularCov (Matrix3x3), linearAcc (Vec3), linearCov (Matrix3x3)
    double data[37];
    for (int i = 0; i < 37; ++i) {
        ifs.read((char*)&(data[i]), sizeof(char) * 8);
    }

    return datalen + 4;
}

int ParseBag::readImage(std::ifstream& ifs) {
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int datalen = toInt(clen, 0, 4);

    // header
    ifs.read((char*)&buffer, sizeof(char) * 4); // seq
    ifs.read((char*)&buffer, sizeof(char) * 8); // time

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // frame_id len
    int frameIdLen = toInt(clen, 0, 4);
    if (frameIdLen > 0) {
        memset(buffer, 0, MAX_LEN);
        ifs.read((char*)&buffer, sizeof(char) * frameIdLen); // frame_id
    }

    // format
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // format len
    int formatLen = toInt(clen, 0, 4);
    memset(buffer, 0, MAX_LEN);
    ifs.read((char*)&buffer, sizeof(char) * formatLen); // format
    std::string format = std::string((char*)buffer);

    // data
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // format len
    int rawDataLen = toInt(clen, 0, 4);
    
    memset(buffer, 0, MAX_LEN);
    ifs.read((char*)&buffer, sizeof(char) * rawDataLen); // data
    std::vector<uchar> imgdata;
    for (int i = 0; i < rawDataLen; ++i) {
        imgdata.push_back(buffer[i]);
    }

    //cv::Mat image = cv::imdecode(imgdata, CV_LOAD_IMAGE_COLOR);
    //if (image.data != nullptr) {
    //    cv::imshow("win", image);
    //    cv::waitKey(0);
    //}

    return datalen + 4;
}

int ParseBag::readPoint(std::ifstream& ifs) {
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int datalen = toInt(clen, 0, 4);

    // header
    ifs.read((char*)&buffer, sizeof(char) * 4); // seq
    ifs.read((char*)&buffer, sizeof(char) * 8); // time

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // frame_id len
    int frameIdLen = toInt(clen, 0, 4);
    memset(buffer, 0, MAX_LEN);
    ifs.read((char*)&buffer, sizeof(char) * frameIdLen); // frame_id

    // height and width 
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int height = toInt(clen, 0, 4);
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int width = toInt(clen, 0, 4);

    // point field
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // point field num
    int fieldNum = toInt(clen, 0, 4);

    for (int i = 0; i < fieldNum; ++i) {
        memset(clen, 0, 4);
        ifs.read((char*)&clen, sizeof(char) * 4); // point field len
        int fieldlen = toInt(clen, 0, 4);

        memset(buffer, 0, MAX_LEN);
        ifs.read((char*)&buffer, sizeof(char) * fieldlen); // field name

        memset(clen, 0, 4);
        ifs.read((char*)&clen, sizeof(char) * 4); // offset
        int offset = toInt(clen, 0, 4);

        memset(clen, 0, 4);
        ifs.read((char*)&clen, sizeof(char) * 1); // datatype
        int datatype = toInt(clen, 0, 4);

        memset(clen, 0, 4);
        ifs.read((char*)&clen, sizeof(char) * 4); // count
        int count = toInt(clen, 0, 4);

        //std::cout << "field " << std::string((char*)buffer) << ":" << "offset " << offset << ", datatype " << datatype <<  ", count " << count << std::endl;
    }

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 1); // is_bigendian

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // point_step
    int pointStep = toInt(clen, 0, 4);

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4); // row_step
    int rowStep = toInt(clen, 0, 4);

    // data
    {
        memset(clen, 0, 4);
        ifs.read((char*)&clen, sizeof(char) * 4); // datalen = height * row_step

        for (int i = 0; i < height; ++i) {
            memset(buffer, 0, MAX_LEN);
            //ifs.read((char*)&buffer, sizeof(char) * rowStep);
            float pos[3] = { 0 }, intensity;
            long float timestemp;
            short ring;
            //std::strncpy((char*)&pos[0], (char*)&buffer, 4);
            ifs.read((char*)&pos[0], sizeof(float));
            ifs.read((char*)&pos[1], sizeof(float));
            ifs.read((char*)&pos[2], sizeof(float));
            ifs.read((char*)&intensity, sizeof(char));
            ifs.read((char*)&ring, sizeof(char) * 2);
            ifs.read((char*)&timestemp, sizeof(char) * 8);
        }
    }

    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 1); // is_dense

    return datalen + 4;
}


int ParseBag::readHeader(std::ifstream& ifs, bool bBagHeader) {
    mapHeaderAttribute.clear();
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int headerlen = toInt(clen, 0, 4);
    if (headerlen <= 0) return -1;
    memset(buffer, 0, MAX_LEN);
    ifs.read((char*)&buffer, sizeof(char) * (bBagHeader ? (4096 + 4) : headerlen));
    int curlen = 0;
    mapHeaderAttribute.clear();
    while (curlen < headerlen) {
        int fieldlen = toInt(buffer, curlen, 4);
        curlen += 4;
        std::string name, value;
        int val = toString(buffer, curlen, fieldlen, name, value);
        curlen += fieldlen;
        if (val < 0) {
            mapHeaderAttribute[name] = value;
        }
        else {
            mapHeaderAttribute[name] = std::to_string(val);
        }
        //std::cout << name << " = " << mapHeaderAttribute[name] << std::endl;
    }
    return headerlen + 4;
}

int ParseBag::readData(std::ifstream& ifs) {
    memset(clen, 0, 4);
    ifs.read((char*)&clen, sizeof(char) * 4);
    int datalen = toInt(clen, 0, 4);
    memset(buffer, 0, MAX_LEN);
    int curlen = 0;
    while (curlen <= datalen - MAX_LEN) {
        ifs.read((char*)&buffer, sizeof(char) * MAX_LEN);
        curlen += MAX_LEN;
    }
    if (curlen < datalen) {
        ifs.read((char*)&buffer, sizeof(char) * (datalen - curlen));
    }
    return datalen + 4;
}

int ParseBag::readHeader(std::FILE* file) {
    memset(clen, 0, 4);
    fread(clen, sizeof(char), 4, file);
    int headerlen = toInt(clen, 0, 4);
    memset(buffer, 0, MAX_LEN);
    fread(buffer, sizeof(char), headerlen, file);
    int curlen = 0;
    mapHeaderAttribute.clear();
    while (curlen < headerlen) {
        int fieldlen = toInt(buffer, curlen, 4);
        curlen += 4;
        std::string name, value;
        int val = toString(buffer, curlen, fieldlen, name, value);
        curlen += fieldlen;
        if (val < 0) {
            mapHeaderAttribute[name] = value;
        }
        else {
            mapHeaderAttribute[name] = std::to_string(val);
        }
        std::cout << name << " = " << mapHeaderAttribute[name] << std::endl;
    }
    return headerlen + 4;
}