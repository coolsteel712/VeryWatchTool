#include "make_watch_face.h"
#include "file_tool.h"
#include "bitmap_tool.h"
#include "fastlz_util.h"
#include "error.h"
#include "debug.h"
#include "json/json.h"
#include "protocol_func_table.h"

using namespace std;

#define MAKE_FILE_IWF_FAILED     1
#define PIXEL_BUFF_MAX_SIZE        (1024*1024)

mkWatchFace::mkWatchFace() {
    file_buff_offset = 0;
    file_head_size = 0;
}


uint32_t mkWatchFace::addtoBuff(const char *string, uint8_t *buff, uint32_t size) {
    if (size == 0) {
        return SUCCESS;
    }

    watch_face_file_data_t data;
    memset(&data, 0, sizeof(data));
    data.data = (uint8_t *) malloc(size);
    data.file_name_length = strlen(string);
    memcpy(data.data, buff, size);
    data.file.offset = file_buff_offset;
    data.file.length = size;
    strncpy(data.file.name, string, WATCH_FACE_RW_FILE_NAME_MAX_LENGTH);

    fileVector.push_back(data);

    file_buff_offset += size;
    file_head_size += sizeof(watch_face_file_t);

    return SUCCESS;
}

uint32_t mkWatchFace::addResItem(string resName, uint8_t format) {
    uint32_t buff_size;
    uint8_t *pixel_buff = (uint8_t *) malloc(PIXEL_BUFF_MAX_SIZE);

    char filePathName[1024];

    //资源文件只支持png
    snprintf(filePathName, sizeof(filePathName), "%s%s", filePath.c_str(), resName.c_str());

    buff_size = bitmap_tool_get_font_buff(filePathName, pixel_buff, PIXEL_BUFF_MAX_SIZE, format);
    if (buff_size != 0) {
        addtoBuff(resName.c_str(), pixel_buff, buff_size);
    }
    free(pixel_buff);

    return SUCCESS;

}

uint32_t mkWatchFace::addFontItem(string fontName, string fileName, uint8_t fontFormat) {
    uint32_t buff_size;
    uint8_t *pixel_buff = (uint8_t *) malloc(PIXEL_BUFF_MAX_SIZE);

    char numFileName[1024];
    char numName[1024];


    char symbol[100];
    char fileSuffix[100];

    if (fileTool::separateFileName(fileName.c_str(), symbol, fileSuffix) == false) {
        return ERROR_INVALID_DATA;
    }

    //添加冒号
#ifdef WIN32
    snprintf(numFileName, sizeof(numFileName), "%s%s\\%s.%s", filePath.c_str(),fontName.c_str(), symbol, fileSuffix);
#else
    snprintf(numFileName, sizeof(numFileName), "%s%s/%s.%s", filePath.c_str(), fontName.c_str(), symbol, fileSuffix);
#endif
    buff_size = bitmap_tool_get_font_buff(numFileName, pixel_buff, PIXEL_BUFF_MAX_SIZE, fontFormat);
    if (buff_size != 0) {
        snprintf(numName, sizeof(numName), "%s_%s", fontName.c_str(), symbol);
        addtoBuff(numName, pixel_buff, buff_size);
    }
    free(pixel_buff);
    return SUCCESS;
}


uint32_t mkWatchFace::addSubResource(std::string filePath, std::string subName, uint8_t format) {
    vector<string> fileList;
    fileTool::getAllFiles(filePath + subName, fileList);
    uint8_t format_temp = format;
    if (fileList.size() == 0) {
        DEBUG_INFO("add res error not find file");
    }

    regex PngBmpFileSuffix("(.*)(.bmp|BMP|png|PNG)"); //PNG和BMP后缀过滤
    regex JpgFileSuffix("(.*)(.jpg|JPG|jpeg|JPEG)"); //JPG后缀过滤
    regex AllSupportFileSuffix("(.*)(.bmp|BMP|png|PNG|jpg|JPG|jpeg|JPEG)"); //所有支持的图片后缀
//	regex fileSuffix("(.*)(.bmp|BMP|png|PNG)");//匹配后缀

    for (int i = 0; i < fileList.size(); i++) {
        string fileName = fileList.at(i);
        //匹配后缀
        //2024-4-17新增jpg图片，使用功能表判断
        if (protocol_func_support_make_watch_dial_decode_jpg()) {
            if (std::regex_match(fileName, AllSupportFileSuffix) == false)
                continue;
        } else {
            if (std::regex_match(fileName, PngBmpFileSuffix) == false)
                continue;
        }

        if (protocol_func_support_make_watch_dial_decode_jpg() && std::regex_match(fileName, JpgFileSuffix)) {
            format_temp = PICTURE_FORMAT_JPG;
        }
        addFontItem(subName, fileName, format_temp);
    }
    return SUCCESS;
}

uint32_t mkWatchFace::addAllResource(std::string filePath, uint8_t format) {
    vector<string> fileList;
    fileTool::getAllFiles(filePath, fileList);

    if (fileList.size() == 0) {
        DEBUG_INFO("add res error not find file");
    }

    regex PngBmpFileSuffix("(.*)(.bmp|BMP|png|PNG)"); //PNG和BMP后缀过滤
    regex JpgFileSuffix("(.*)(.jpg|JPG|jpeg|JPEG)"); //JPG后缀过滤
    regex AllSupportFileSuffix("(.*)(.bmp|BMP|png|PNG|jpg|JPG|jpeg|JPEG)"); //所有支持的图片后缀
//	regex fileSuffix("(.*)(.bmp|BMP|png|PNG)");//匹配后缀

    for (int i = 0; i < fileList.size(); i++) {
        string fileName = fileList.at(i);
        //匹配后缀
        //2024-4-17新增jpg图片，使用功能表判断
        if (protocol_func_support_make_watch_dial_decode_jpg()) {
            if (std::regex_match(fileName, AllSupportFileSuffix) == false)
                continue;
        } else {
            if (std::regex_match(fileName, PngBmpFileSuffix) == false)
                continue;
        }

        if (protocol_func_support_make_watch_dial_decode_jpg() && std::regex_match(fileName, JpgFileSuffix)) {
            addResItem(fileName, PICTURE_FORMAT_JPG); //JPG图片
        } else if (fileName.find("_4bit") != string::npos) {
            addResItem(fileName, FONT_FORMAT_MONO4);
        } else {
            addResItem(fileName, format);
        }

    }
    return SUCCESS;
}

uint32_t mkWatchFace::makeFile(std::string filePath, std::string saveFileName, uint8_t format) {
    fstream outFile;
    outFile.open(filePath + saveFileName, ios::out | ios::trunc | ios::binary);

    this->filePath = filePath;
    char fileBuff[100 * 1024] = {0};

    fstream file;

    file.open(filePath + WATCH_FACE_RW_FILE_MAIN_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();
    LOG_ERROR("read main %s", fileBuff);

    Json::CharReaderBuilder reader;
    Json::Value root;
    string errorString;
    std::unique_ptr<Json::CharReader> const jsonReader(reader.newCharReader());
    bool ret = jsonReader->parse(fileBuff, fileBuff + strlen(fileBuff), &root, &errorString);
    if (ret == false) {
        LOG_INFO("err:file %s json parse fail", WATCH_FACE_RW_FILE_MAIN_FILE_NAME);
//        return ERROR_INVALID_DATA;
    }
    string preview = root["preview"].asString();
    string name = root["name"].asString();
    string author = root["author"].asString();
    string description = root["description"].asString();

//	int version = root["version"].asInt();
    Json::Value IwfItem = root["item"];
    if (!strcmp("LZ4", root["compress"].asString().c_str()) && protocol_func_support_lvgl_watch_dial_frame())
        compress_type = LZ4_TYPE;
    else if (!strcmp("FASTLZ", root["compress"].asString().c_str()) &&
             protocol_func_support_douiv6_watch_dial_anima_compress_pixel_data())
        compress_type = FASTLZ_TYPE;
    else
        compress_type = NO_TYPE;
    DEBUG_INFO("watch name:%s compress type:%d", name.c_str(), compress_type);

    //添加主描述json文件
    addtoBuff(WATCH_FACE_RW_FILE_MAIN_FILE_NAME, (uint8_t *) fileBuff, (uint32_t) strlen(fileBuff));

    //添加主文件1
    memset(fileBuff, 0, sizeof(fileBuff));
    file.open(filePath + WATCH_FACE_RW_FILE_MAIN1_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();
    LOG_ERROR("read main1 %s", fileBuff);
    int main1_length = (int) strlen(fileBuff);
    if (main1_length != 0) {
        addtoBuff(WATCH_FACE_RW_FILE_MAIN1_FILE_NAME, (uint8_t *) fileBuff, (uint32_t) strlen(fileBuff));
    }

    uint8_t *pixel_buff = (uint8_t *) malloc(PIXEL_BUFF_MAX_SIZE);
//	uint32_t buff_size;

    //添加字体描述文件
    memset(fileBuff, 0, sizeof(fileBuff));
    file.open(filePath + WATCH_FACE_RW_FILE_FONT_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();
    LOG_ERROR("read font %s", fileBuff);

    ret = jsonReader->parse(fileBuff, fileBuff + strlen(fileBuff), &root, &errorString);
    if (ret == false) {
        DEBUG_INFO("warn:file %s json parse fail,json buf:%s", WATCH_FACE_RW_FILE_FONT_FILE_NAME, fileBuff);
    }
    addtoBuff(WATCH_FACE_RW_FILE_FONT_FILE_NAME, (uint8_t *) fileBuff, (uint32_t) strlen(fileBuff));
    Json::Value FontItem = root["item"];

    //添加根目录下资源
    addAllResource(filePath, format);

    string fontName;
    int32_t bpp;
    for (int i = 0; i < FontItem.size(); i++) {
        fontName = FontItem[i]["name"].asString();
        bpp = FontItem[i]["bpp"].asInt();
        if (fontName.length() == 0) {
            continue;
        }
        //添加子目录
        if (bpp == 4) {
            addSubResource(filePath, fontName, FONT_FORMAT_MONO4);
        } else {
            addSubResource(filePath, fontName, format);
        }

    }

    //写头部
    watch_face_file_head_t file_head;
    memset(&file_head, 0, sizeof(file_head));

    strcpy(file_head.magic, WATCH_FACE_RW_FILE_MAGIC);
    file_head.version = 1;
    file_head.file_count = fileVector.size();
    LOG_ERROR("file_count = %d", file_head.file_count);

    outFile.write((const char *) &file_head, sizeof(file_head));

    file_head_size += sizeof(file_head);

    //写头部
    for (int i = 0; i < fileVector.size(); i++) {

        watch_face_file_data_t data = fileVector.at(i);
        data.file.offset += file_head_size;
        outFile.write((char *) &data.file, sizeof(data.file));
    }
    //写详情
    for (int i = 0; i < fileVector.size(); i++) {

        watch_face_file_data_t data = fileVector.at(i);
        if (data.file_name_length >= WATCH_FACE_RW_FILE_NAME_MAX_LENGTH) {
            data.file_name_length = WATCH_FACE_RW_FILE_NAME_MAX_LENGTH - 1;
        }
        data.file.name[data.file_name_length] = '\0';
        LOG_ERROR("write data : i = %d,file = %s,offset = %d,size = %d", i, data.file.name, data.file.offset,
                  data.file.length);
        outFile.seekp(file_head_size + data.file.offset, ios::beg);
        outFile.write((char *) data.data, data.file.length);
        free(data.data);
    }

    outFile.close();

    LOG_ERROR("%s write success", saveFileName.c_str());

    free(pixel_buff);

    return SUCCESS;
}

uint32_t mkWatchFace::makeIwfFile(std::string filePath, std::string saveFilePath, uint8_t format) {
    fstream outFile;
    outFile.open(saveFilePath, ios::out | ios::trunc | ios::binary);

    this->filePath = filePath;
    char fileBuff[100 * 1024] = {0};

    fstream file;
    printf("file: %s\n",(filePath.c_str()));
    file.open(filePath + WATCH_FACE_RW_FILE_MAIN_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();
    LOG_ERROR("read main %s", fileBuff);

    Json::CharReaderBuilder reader;
    Json::Value root;
    string errorString;
    std::unique_ptr<Json::CharReader> const jsonReader(reader.newCharReader());
    bool ret = jsonReader->parse(fileBuff, fileBuff + strlen(fileBuff), &root, &errorString);
    if (ret == false) {
        printf("warn:file parse fail");
    }
    string preview = root["preview"].asString();
    string name = root["name"].asString();
    string author = root["author"].asString();
    string description = root["description"].asString();
    int version = root["version"].asInt();

    Json::Value IwfItem = root["item"];
    if (!strcmp("LZ4", root["compress"].asString().c_str()) && protocol_func_support_lvgl_watch_dial_frame())
        compress_type = LZ4_TYPE;
    else if (!strcmp("FASTLZ", root["compress"].asString().c_str()) &&
             protocol_func_support_douiv6_watch_dial_anima_compress_pixel_data())
        compress_type = FASTLZ_TYPE;
    else
        compress_type = NO_TYPE;

    DEBUG_INFO("watch name:%s compress type:%d", name.c_str(), compress_type);


    //添加主描述json文件
    addtoBuff(WATCH_FACE_RW_FILE_MAIN_FILE_NAME, (uint8_t *) fileBuff, strlen(fileBuff));


    //添加主文件1
    memset(fileBuff, 0, sizeof(fileBuff));
    file.open(filePath + WATCH_FACE_RW_FILE_MAIN1_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();
    LOG_ERROR("read main1 %s", fileBuff);

    int main1_length = (int) strlen(fileBuff);
    if (main1_length != 0) {
        addtoBuff(WATCH_FACE_RW_FILE_MAIN1_FILE_NAME, (uint8_t *) fileBuff, strlen(fileBuff));
    }

    uint8_t *pixel_buff = (uint8_t *) malloc(PIXEL_BUFF_MAX_SIZE);

    //添加字体描述文件
    memset(fileBuff, 0, sizeof(fileBuff));
    file.open(filePath + WATCH_FACE_RW_FILE_FONT_FILE_NAME, ios::in | ios::binary);
    file.read(fileBuff, sizeof(fileBuff));
    file.close();

    ret = jsonReader->parse(fileBuff, fileBuff + strlen(fileBuff), &root, &errorString);
    if (ret == false) {
        DEBUG_INFO("warn:file parse fail");
    }
    addtoBuff(WATCH_FACE_RW_FILE_FONT_FILE_NAME, (uint8_t *) fileBuff, (uint32_t) strlen(fileBuff));
    Json::Value FontItem = root["item"];

    //添加根目录下资源
    addAllResource(filePath, format);

    string fontName;
    int32_t bpp;
    for (int i = 0; i < FontItem.size(); i++) {
        fontName = FontItem[i]["name"].asString();
        bpp = FontItem[i]["bpp"].asInt();

        if (fontName.length() == 0) {
            continue;
        }
        //添加子目录
        if (bpp == 4) {
            addSubResource(filePath, fontName, FONT_FORMAT_MONO4);
        } else {
            addSubResource(filePath, fontName, format);
        }

    }

    //写头部
    watch_face_file_head_t file_head;
    memset(&file_head, 0, sizeof(file_head));

    strcpy(file_head.magic, WATCH_FACE_RW_FILE_MAGIC);
    file_head.version = 1;
    file_head.file_count = fileVector.size();
    LOG_ERROR("file_count = %d", file_head.file_count);

    outFile.write((const char *) &file_head, sizeof(file_head));

    file_head_size += sizeof(file_head);

    //写头部
    for (int i = 0; i < fileVector.size(); i++) {

        watch_face_file_data_t data = fileVector.at(i);
        data.file.offset += file_head_size;
        outFile.write((char *) &data.file, sizeof(data.file));
    }

    //写详情
    for (int i = 0; i < fileVector.size(); i++) {

        watch_face_file_data_t data = fileVector.at(i);
        if (data.file_name_length >= WATCH_FACE_RW_FILE_NAME_MAX_LENGTH) {
            data.file_name_length = WATCH_FACE_RW_FILE_NAME_MAX_LENGTH - 1;
        }
        data.file.name[data.file_name_length] = '\0';
        LOG_ERROR("write data : i = %d,file = %s,offset = %d,size = %d", i, data.file.name, data.file.offset,
                  data.file.length);
        outFile.seekp(file_head_size + data.file.offset, ios::beg);
        outFile.write((char *) data.data, data.file.length);
        free(data.data);
    }

    uint32_t filelen = (uint32_t) outFile.tellp();
    outFile.close();
    LOG_ERROR("%s write success, filelen = %d", saveFilePath.c_str(), filelen);

    free(pixel_buff);

    return filelen;
}

uint32_t
mkWatchFace::makeCompressionFile(std::string filePath, std::string saveFileName, uint8_t format, uint16_t block_size) {
    uint32_t err_code;
    LOG_ERROR("makeCompressionFile filePath = %s,saveFileName = %s,format = %d,block_size = %d", filePath.c_str(),
              saveFileName.c_str(), format, block_size);

    if (block_size == 0) {
        block_size = 4096;
        DEBUG_INFO("warn:block size is zero default set 4096");
    }

    err_code = makeFile(filePath, saveFileName, format);
    if (err_code != SUCCESS) {
        return err_code;
    }

    string inName = filePath + saveFileName;

    FILE *in = fopen(inName.c_str(), "rb");
    if (in == NULL) {
        DEBUG_INFO("ERROR %s open fail", inName.c_str());
        return ERROR_NULL;
    }


    string outName = filePath + saveFileName + ".lz";
    LOG_ERROR("outName:%s", outName.c_str());
    FILE *out = fopen(outName.c_str(), "wb+");

    if (out == NULL) {
        DEBUG_INFO("ERROR %s open fail", outName.c_str());
        fclose(in);
        return ERROR_NULL;
    }
    int ret = fastlz_util_compress_file(in, out, block_size);
    fclose(in);
    fclose(out);
    return ret;

}
