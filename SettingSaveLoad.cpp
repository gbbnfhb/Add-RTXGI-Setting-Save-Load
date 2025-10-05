#include <donut/core/math/math.h>
#include <donut/app/ApplicationBase.h>
#include <donut/app/UserInterfaceUtils.h>
#include <donut/engine/Scene.h>
#include <donut/engine/TextureCache.h>

#include "Pathtracer.h"

#include "PathtracerUI.h"
#include <fstream>
#include <json/json.h>


#define LOAD_BOOL(key, var)   (var) = root.get((key), (var)).asBool()
#define LOAD_INT(key, var)    (var) = root.get((key), (var)).asInt()
#define LOAD_FLOAT(key, var)  (var) = root.get((key), (var)).asFloat()
#define LOAD_ENUM(key, var)   (var) = static_cast<decltype(var)>(root.get((key), static_cast<int>(var)).asInt())


/**
 * @brief 現在のUI設定をJSONファイルに保存します。
 * @param filename 保存先のファイルパス。
 * @return 保存に成功した場合はtrue、失敗した場合はfalse。
 */
bool PathtracerUI::SaveSettings(const std::string& filename)
{
    Json::Value root;
    
    // Generic
    root["VSync"] = m_ui.enableVSync;
    root["Jitter"] = m_ui.enableJitter;
    root["RussianRoulette"] = m_ui.enableRussianRoulette;
    root["Transmission"] = m_ui.enableTransmission;
    root["Animations"] = m_ui.enableAnimations;

    // Path Tracing
    root["techSelection"] = static_cast<int>(m_ui.techSelection);
    root["denoiserSelection"] = static_cast<int>(m_ui.denoiserSelection);
    root["bouncesMax"] = m_ui.bouncesMax;
    root["samplesPerPixel"] = m_ui.samplesPerPixel;
    root["exposureAdjustment"] = m_ui.exposureAdjustment;
    root["roughnessMin"] = m_ui.roughnessMin;
    root["roughnessMax"] = m_ui.roughnessMax;
    root["metalnessMin"] = m_ui.metalnessMin;
    root["metalnessMax"] = m_ui.metalnessMax;
    root["ptDebugOutput"] = static_cast<int>(m_ui.ptDebugOutput);

#if ENABLE_NRC
    // NRC
    root["nrcTrainCache"] = m_ui.nrcTrainCache;
    root["nrcTrainingBounces"] = m_ui.nrcMaxTrainingBounces;
    root["Learn Irradiance"] = m_ui.nrcLearnIrradiance;
    root["Include Direct Illumination"] = m_ui.nrcIncludeDirectIllumination;
    root["Skip delta vertices"] = m_ui.nrcSkipDeltaVertices;
    root["Self-Training Attenuation"] = m_ui.nrcSelfTrainingAttenuation;
    root["Heuristic Threshold"] = m_ui.nrcTerminationHeuristicThreshold;
    root["Num Training Iterations"] = m_ui.nrcNumTrainingIterations,1,4;
    root["Primary segments to train on"] = m_ui.nrcProportionPrimarySegmentsToTrainOn;
    root["Tertiary+ segments to train on"] = m_ui.nrcProportionTertiaryPlusSegmentsToTrainOn;
    root["Proportion unbiased"] = m_ui.nrcProportionUnbiased;
    root["Unbiased self-training"] = m_ui.nrcProportionUnbiasedToSelfTrain;
    root["Max Average Radiance Value"] = m_ui.nrcMaxAverageRadiance;
    root["Resolve Mode"] = static_cast<int>(m_ui.nrcResolveMode);

#endif

#if ENABLE_SHARC
    // SHaRC
    root["sharcEnableClear"] = m_ui.sharcEnableClear;
    root["Enable Update"] = m_ui.sharcEnableUpdate;
    root["Enable Resolve"] = m_ui.sharcEnableResolve;
    root["Enable Anti Firefly"] = m_ui.sharcEnableAntiFireflyFilter;
    root["Update View Camera"] = m_ui.sharcUpdateViewCamera;
    root["Enable Material Demodulation"] = m_ui.sharcEnableMaterialDemodulation;
    root["Enable Debug"] = m_ui.sharcEnableDebug;
    root["Accumulation Frame Number"] = m_ui.sharcAccumulationFrameNum;
    root["Stale Frame Number"] = m_ui.sharcStaleFrameFrameNum;
    root["Downscale Factor"] = m_ui.sharcDownscaleFactor;
    root["Scene Scale"] = m_ui.sharcSceneScale;
    root["Rougness Threshold"] = m_ui.sharcRoughnessThreshold;

#endif

    // Lighting
    root["enableSky"] = m_ui.enableSky;
    root["skyColor"][0] = m_ui.skyColor[0];
    root["skyColor"][1] = m_ui.skyColor[1];
    root["skyColor"][2] = m_ui.skyColor[2];
    root["skyColor"][3] = m_ui.skyColor[3];
    root["skyIntensity"] = m_ui.skyIntensity;
    root["enableEmissives"] = m_ui.enableEmissives;
    root["enableLighting"] = m_ui.enableLighting;

    // Tone mapping
    root["toneMappingOperator"] = static_cast<int>(m_ui.toneMappingOperator);
    root["toneMappingClamp"] = m_ui.toneMappingClamp;

    // ファイルに書き出し
    std::ofstream ofs(filename);
    if (!ofs) {
        // エラー処理: ファイルが開けなかった
        return false;
    }

    Json::StreamWriterBuilder writer;
    writer["commentStyle"] = "None";
    writer["indentation"] = "  "; // 見やすいようにインデントを設定
    std::unique_ptr<Json::StreamWriter> jsonWriter(writer.newStreamWriter());
    jsonWriter->write(root, &ofs);

    return true;
}

/**
 * @brief JSONファイルからUI設定を読み込みます。
 * @param filename 読み込み元のファイルパス。
 * @return 読み込みに成功した場合はtrue、失敗した場合はfalse。
 */
bool PathtracerUI::LoadSettings(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs) {
        // エラー処理: ファイルが存在しない、または開けない
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, ifs, &root, &errs)) {
        // エラー処理: JSONのパースに失敗
        return false;
    }

    // JSONオブジェクトから値を取得し、m_uiに設定
    // キーが存在しない場合に備え、デフォルト値付きの.get()を使うとより安全
    LOAD_BOOL("VSync",m_ui.enableVSync);
    LOAD_BOOL("Jitter",m_ui.enableJitter);
    LOAD_BOOL("RussianRoulette",m_ui.enableRussianRoulette);
    LOAD_BOOL("Transmission",m_ui.enableTransmission);
    LOAD_BOOL("Animations",m_ui.enableAnimations);

    // Path Tracing
    LOAD_ENUM("techSelection",m_ui.techSelection);
    LOAD_ENUM("denoiserSelection",m_ui.denoiserSelection);
    LOAD_INT("bouncesMax",m_ui.bouncesMax);
    LOAD_INT("samplesPerPixel",m_ui.samplesPerPixel);
    LOAD_FLOAT("exposureAdjustment",m_ui.exposureAdjustment);
    LOAD_FLOAT("roughnessMin",m_ui.roughnessMin);
    LOAD_FLOAT("roughnessMax",m_ui.roughnessMax);
    LOAD_FLOAT("metalnessMin",m_ui.metalnessMin);
    LOAD_FLOAT("metalnessMax",m_ui.metalnessMax);
    LOAD_ENUM("ptDebugOutput",m_ui.ptDebugOutput);

#if ENABLE_NRC
    // NRC
    LOAD_BOOL("nrcTrainCache",m_ui.nrcTrainCache);
    LOAD_INT("nrcTrainingBounces",m_ui.nrcMaxTrainingBounces);
    LOAD_BOOL("Learn Irradiance",m_ui.nrcLearnIrradiance);
    LOAD_BOOL("Include Direct Illumination",m_ui.nrcIncludeDirectIllumination);
    LOAD_BOOL("Skip delta vertices",m_ui.nrcSkipDeltaVertices);
    LOAD_FLOAT("Self-Training Attenuation",m_ui.nrcSelfTrainingAttenuation);
    LOAD_FLOAT("Heuristic Threshold",m_ui.nrcTerminationHeuristicThreshold);
    LOAD_INT("Num Training Iterations",m_ui.nrcNumTrainingIterations);
    LOAD_FLOAT("Primary segments to train on",m_ui.nrcProportionPrimarySegmentsToTrainOn);
    LOAD_FLOAT("Tertiary+ segments to train on",m_ui.nrcProportionTertiaryPlusSegmentsToTrainOn);
    LOAD_FLOAT("Proportion unbiased",m_ui.nrcProportionUnbiased);
    LOAD_FLOAT("Unbiased self-training",m_ui.nrcProportionUnbiasedToSelfTrain);
    LOAD_FLOAT("Max Average Radiance Value",m_ui.nrcMaxAverageRadiance);
    LOAD_ENUM("Resolve Mode",m_ui.nrcResolveMode);

#endif

#if ENABLE_SHARC
    // SHaRC
    LOAD_BOOL("Enable Clear", m_ui.sharcEnableClear);
    LOAD_BOOL("Enable Update", m_ui.sharcEnableUpdate);
    LOAD_BOOL("Enable Resolve", m_ui.sharcEnableResolve);
    LOAD_BOOL("Enable Anti Firefly", m_ui.sharcEnableAntiFireflyFilter);
    LOAD_BOOL("Update View Camera", m_ui.sharcUpdateViewCamera);
    LOAD_BOOL("Enable Material Demodulation", m_ui.sharcEnableMaterialDemodulation);
    LOAD_BOOL("Enable Debug", m_ui.sharcEnableDebug);
    LOAD_INT("Accumulation Frame Number", m_ui.sharcAccumulationFrameNum);
    LOAD_INT("Stale Frame Number", m_ui.sharcStaleFrameFrameNum);
    LOAD_INT("Downscale Factor", m_ui.sharcDownscaleFactor);
    LOAD_FLOAT("Scene Scale", m_ui.sharcSceneScale);
    LOAD_FLOAT("Rougness Threshold", m_ui.sharcRoughnessThreshold);
#endif

    // Lighting
    m_ui.enableSky = root.get("enableSky", m_ui.enableSky).asBool();
    const Json::Value& skyColorJson = root["skyColor"];
    if (skyColorJson.isArray() && skyColorJson.size() == 4) {
        m_ui.skyColor[0] = skyColorJson[0].asFloat();
        m_ui.skyColor[1] = skyColorJson[1].asFloat();
        m_ui.skyColor[2] = skyColorJson[2].asFloat();
        m_ui.skyColor[3] = skyColorJson[3].asFloat();
    }
    LOAD_FLOAT("skyIntensity", m_ui.skyIntensity);
    LOAD_BOOL("enableEmissives", m_ui.enableEmissives);
    LOAD_BOOL("enableLighting", m_ui.enableLighting);

    // Tone mapping
    LOAD_ENUM("toneMappingOperator", m_ui.toneMappingOperator);
    LOAD_BOOL("toneMappingClamp", m_ui.toneMappingClamp);

    // 設定を読み込んだ後、レンダリング状態をリセットする必要がある
    m_app.ResetAccumulation();

    return true;
}