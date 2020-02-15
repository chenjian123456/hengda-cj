/**
 * ============================================================================
 *
 * Copyright (C) 2019, Huawei Technologies Co., Ltd. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1 Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   2 Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   3 Neither the names of the copyright holders nor the names of the
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */
#include "SSDDetection.h"
#include "dvpp_utils.h"
#include "engine_tools.h"
#include "error_code.h"
#include "hiaiengine/ai_memory.h"
#include "hiaiengine/c_graph.h"
#include "hiaiengine/data_type.h"
#include "hiaiengine/log.h"
#include "opencv2/opencv.hpp"
#include <memory>
#include "base/log.h"

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;
#define uchar unsigned char

typedef struct rectangle{
    int x;
    int y;
    int w;
    int h;
    uchar thin;
    uchar YUV[3];
}RECT;

void RgbToYuv(uchar RGB[3], uchar YUV[3])
{
    /* RGB convert YUV */
    YUV[0] =  0.299  * RGB[0] + 0.587  * RGB[1] + 0.114  * RGB[2];
    YUV[1] = -0.1687 * RGB[0] + 0.3313 * RGB[1] + 0.5    * RGB[2] + 128;
    YUV[2] =  0.5    * RGB[0] - 0.4187 * RGB[1] - 0.0813 * RGB[2] + 128;
}

void FindAxis(RECT rect, int axis[])
{
    axis[0] = axis[2] = axis[4] = rect.x;
    axis[6] = rect.x + rect.w - rect.thin;
    axis[1] = rect.y;
    axis[3] = rect.y + rect.h - rect.thin;
    axis[5] = axis[7] = rect.y + rect.thin;
}

void DrawWidthLine(uchar* pic, int pic_w, int pic_h, RECT rect, int axis[])
{
    int i, j, k;
    int y_index, u_index, v_index;
    for (i = 0; i < 4; i += 2){
        for (j = axis[i+1]+rect.thin-1; j >= axis[i+1]; j--){
            for (k = axis[i]+rect.w-1; k >= axis[i]; k--){

                y_index = j * pic_w + k;
                u_index = ((j >> 1) + pic_h) * pic_w + k - (k & 1);
                //u_index = (((y_index >> 1) - (pic_w >> 1) * ((j + 1) >> 1)) << 1) + pic_w * pic_h;
                v_index = u_index + 1;

                pic[y_index] = rect.YUV[0];
                pic[u_index] = rect.YUV[1];
                pic[v_index] = rect.YUV[2];

            }
        }
    }
}

void DrawHeightLine(uchar* pic, int pic_w, int pic_h, RECT rect, int axis[])
{
    int i, j, k;
    int y_index, u_index, v_index;
    for (i = 4; i < 8; i += 2){
        for (k = axis[i]+rect.thin-1; k >= axis[i]; k--){
            for (j = axis[i+1]+rect.h-(2*rect.thin)-1; j >= axis[i+1]; j--){
                y_index = j * pic_w + k;
                u_index = ((j >> 1) + pic_h) * pic_w + k - (k & 1);
                //u_index = (((y_index >> 1) - (pic_w >> 1) * ((j + 1) >> 1)) << 1) + pic_w * pic_h;
                v_index = u_index + 1;

                pic[y_index] = rect.YUV[0];
                pic[u_index] = rect.YUV[1];
                pic[v_index] = rect.YUV[2];

            }
        }
    }
}

void NV12MarkRect(uchar* pic, int pic_w, int pic_h, RECT rect)
{
    int axis[8];

    FindAxis(rect, axis);
    DrawWidthLine(pic, pic_w, pic_h, rect, axis);
    DrawHeightLine(pic, pic_w, pic_h, rect, axis);
}



HIAI_StatusT SSDDetection::Init(const hiai::AIConfig& config,
    const std::vector<hiai::AIModelDescription>& model_desc)
{
    HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] start init!");
    HIAI_StatusT ret = HIAI_OK;
    if (nullptr == modelManager) {
        modelManager = std::make_shared<hiai::AIModelManager>();
    }
    hiai::AIModelDescription modelDesc;
    loadModelDescription(config, modelDesc);
    // init ai model manager
    ret = modelManager->Init(config, { modelDesc });
    if (hiai::SUCCESS != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] ai model manager init failed!");
        return HIAI_ERROR;
    }
    // input/output buffer allocation
    std::vector<hiai::TensorDimension> inputTensorDims;
    std::vector<hiai::TensorDimension> outputTensorDims;
    ret = modelManager->GetModelIOTensorDim(modelDesc.name(), inputTensorDims, outputTensorDims);
    if (ret != hiai::SUCCESS) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] hiai ai model manager init failed.");
        return HIAI_ERROR;
    }
    for (auto& dims : inputTensorDims) {
        logDumpDims(dims);
    }
    for (auto& dims : outputTensorDims) {
        logDumpDims(dims);
    }
    // input dims
    if (1 != inputTensorDims.size()) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] inputTensorDims.size() != 1 (%d vs. %d)",
            inputTensorDims.size(), 1);
        return HIAI_ERROR;
    }

    kBatchSize = inputTensorDims[0].n;
    kChannel = inputTensorDims[0].c;
    kHeight = inputTensorDims[0].h;
    kWidth = inputTensorDims[0].w;
    kInputSize = inputTensorDims[0].size;

    kAlignedWidth = ALIGN_UP(kWidth, DVPP_STRIDE_WIDTH);
    kAlignedHeight = ALIGN_UP(kHeight, DVPP_STRIDE_HEIGHT);


    ret = creatIOTensors(modelManager, inputTensorDims, inputTensorVec, inputDataBuffer);
    if (HIAI_OK != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] creat input tensors failed!");
        return HIAI_ERROR;
    }
    ret = creatIOTensors(modelManager, outputTensorDims, outputTensorVec, outputDataBuffer);
    if (HIAI_OK != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] creat output tensors failed!");
        return HIAI_ERROR;
    }
    HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[SSDDetection] end init!");
    return HIAI_OK;
}

HIAI_IMPL_ENGINE_PROCESS("SSDDetection", SSDDetection, DT_INPUT_SIZE)
{
    HIAI_StatusT ret = HIAI_OK;
    std::shared_ptr<DeviceStreamData> inputArg = std::static_pointer_cast<DeviceStreamData>(arg0);
    if(inputArg == nullptr){
        return HIAI_OK;
    }
    if (nullptr == inputArg) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "Fail to process invalid message");
        return HIAI_ERROR;
    }

    inputArgQueue.push_back(std::move(inputArg));
    // waiting for batch data
    if (inputArgQueue.size() < kBatchSize) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR,
            "Collecting batch data, in queue, current size %d", inputArgQueue.size());
        return HIAI_OK;
    }
    // resize yuv data to input size
    uint8_t* dataBufferPtr = inputDataBuffer[0].get();
    for (int i = 0; i < inputArgQueue.size(); i++) {
        inputArg = inputArgQueue[i];
        char outFilename[128];
        time_pair vpcStamps;
        vpcResize(inputArg->imgOrigin.buf.data.get(), inputArg->imgOrigin.width, inputArg->imgOrigin.height,
            dataBufferPtr, kWidth, kHeight);
        dataBufferPtr += kInputSize;
    }
    // inference
    hiai::AIContext aiContext;
    time_pair process;
    ret = modelManager->Process(aiContext, inputTensorVec, outputTensorVec, 0);
    if (hiai::SUCCESS != ret) {
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "AI Model Manager Process failed");
        return HIAI_ERROR;
    }

    postProcessDetection();
    inputArgQueue.clear();

    return HIAI_OK;
}

HIAI_StatusT SSDDetection::postProcessDetection()
{
    // tensor shape 200x7x1x1
    // for each row (7 elements), layout as follows
    // batch, label, score, xmin, ymin, xmax, ymax
    shared_ptr<hiai::AINeuralNetworkBuffer> tensorResults = std::static_pointer_cast<hiai::AINeuralNetworkBuffer>(outputTensorVec[0]);
    shared_ptr<hiai::AINeuralNetworkBuffer> tensorObjNum = std::static_pointer_cast<hiai::AINeuralNetworkBuffer>(outputTensorVec[1]);
    int objNum = (int)(*(float*)tensorObjNum->GetBuffer());
    const float thresh = 0.5;
    const int colSize = 7;
    int validFaceCount = 0;
    float* resPtr = (float*)tensorResults->GetBuffer();
    
    for (int i = 0; i < objNum; i++) {
        float score = *(resPtr + 2);
        if (score > thresh) {
            int batch = (int)(*resPtr);
            int label = (int)(*(resPtr + 1));
            float xmin = *(resPtr + 3);
            float ymin = *(resPtr + 4);
            float xmax = *(resPtr + 5);
            float ymax = *(resPtr + 6);
            validFaceCount++;
            // batch, label, score, xmin, ymin, xmax, ymax);
            shared_ptr<DeviceStreamData>* streamPtr = &inputArgQueue[batch];
            const uint32_t img_width = (*streamPtr)->imgOrigin.width;
            const uint32_t img_height = (*streamPtr)->imgOrigin.height;
            DetectInfo info;
            info.classId = label;
            info.location.anchor_lt.x = std::max(xmin, 0.f) * img_width;
            info.location.anchor_lt.y = std::max(ymin, 0.f) * img_height;
            info.location.anchor_rb.x = std::min(xmax, 1.f) * img_width;
            info.location.anchor_rb.y = std::min(ymax, 1.f) * img_height;
            info.confidence = score;
            (*streamPtr)->detectResult.push_back(info);

            if(label == 1){
                uchar RGBblue[3] = {255, 0, 0};
                uchar YUVblue[3];
                RgbToYuv(RGBblue, YUVblue);
               
                RECT Rect;
                Rect.x = info.location.anchor_lt.x;
                Rect.y = info.location.anchor_lt.y;
                Rect.w = info.location.anchor_rb.x - Rect.x;
                Rect.h = info.location.anchor_rb.y - Rect.y;
                Rect.thin = 3;
                Rect.YUV[0] = YUVblue[0];
                Rect.YUV[1] = YUVblue[1];
                Rect.YUV[2] = YUVblue[2];
                NV12MarkRect((*streamPtr)->imgOrigin.buf.data.get(), (*streamPtr)->imgOrigin.width, (*streamPtr)->imgOrigin.height, Rect);
            }
        
        }
        resPtr += colSize;

    }
    for (auto& outputData : inputArgQueue) {
        HIAI_StatusT ret = SendData(0, "DeviceStreamData", std::static_pointer_cast<void>(outputData));
        if (HIAI_OK != ret) {
            HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "SSDDetection send data failed");
        }
    }

    return HIAI_OK;

}