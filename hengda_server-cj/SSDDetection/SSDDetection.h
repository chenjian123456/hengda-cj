#ifndef SAMPLE_SSDDETECTION_H
#define SAMPLE_SSDDETECTION_H

#include "dvpp/idvppapi.h"
#include "hiaiengine/ai_model_manager.h"
#include "hiaiengine/engine.h"
#include "stream_data.h"
#include <utility>

#define DT_INPUT_SIZE 1
#define DT_OUTPUT_SIZE 16

class SSDDetection : public hiai::Engine {
public:
    SSDDetection() {}

    HIAI_StatusT Init(const hiai::AIConfig& config,
        const std::vector<hiai::AIModelDescription>& model_desc);

    HIAI_DEFINE_PROCESS(DT_INPUT_SIZE, DT_OUTPUT_SIZE)

private:
    uint32_t kBatchSize = 1;
    uint32_t kChannel = 0;
    uint32_t kWidth = 0;
    uint32_t kHeight = 0;
    uint32_t kAlignedWidth = 0;
    uint32_t kAlignedHeight = 0;
    uint32_t kInputSize = 0;
    
    std::shared_ptr<hiai::AIModelManager> modelManager;
    std::vector<std::shared_ptr<DeviceStreamData> > inputArgQueue;
    std::vector<std::shared_ptr<uint8_t> > inputDataBuffer;
    std::vector<std::shared_ptr<uint8_t> > outputDataBuffer;
    std::vector<std::shared_ptr<hiai::IAITensor> > inputTensorVec;
    std::vector<std::shared_ptr<hiai::IAITensor> > outputTensorVec;

    HIAI_StatusT postProcessDetection();
};

#endif //HENGDA_SSDDETECTION_H
