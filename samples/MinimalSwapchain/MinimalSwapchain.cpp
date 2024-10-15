import NYANData;
import NYANLog;
import NYANVulkan;
import NYANGLFW;

#include <cassert>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <vector>
#include <array>
#include <optional>
#include <string_view>
#include <span>
#include <string>

//#define GLFW_EXPOSE_NATIVE_WIN32
//#define NOMINMAX
//#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>

#include "volk.h"

using namespace nyan;
using namespace nyan::vulkan;

enum class ExitCode : int {
    Success = EXIT_SUCCESS,
    Failure = EXIT_FAILURE
};

int main(const int argc, char const* const* const argv)
{
    [[nodiscard]] ExitCode better_main(std::span<const std::string_view>);
    std::vector<std::string_view>
        args(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));

    return static_cast<int>(better_main(args));
}

static std::vector<uint32_t> read_binary_file(const std::filesystem::path& path)
{

    std::vector<uint32_t> data;
    std::ifstream in(path, std::ios::binary);
    if (in.is_open()) {
        auto size = std::filesystem::file_size(path);
        data.resize(((size-1u) / sizeof(uint32_t)) + 1u);
        in.read(reinterpret_cast<char*>(data.data()), size);
        in.close();
    }
    else {
    }

    return data;
}

inline static std::unique_ptr<Instance> instance{ nullptr };
inline static std::unique_ptr<LogicalDevice> logicalDevice{ nullptr };

bool setup(nyan::glfw::Window& window, nyan::glfw::Library& library) {
    auto resResult = window.get_resolution();
    if (!resResult)
        return false;

    auto [width, height] = *resResult;

    Instance::ValidationSettings validationSettings{
        .enabled {false},
        .createCallback {true},
        .callBackVerbose { true },
        .callBackInfo { true },
        .callBackWarning { true },
        .callBackError { true },
        .callBackGeneral { true },
        .callBackValidation { true },
        .callBackPerformance { true },
        .callBackDeviceAddressBinding { true },
        .gpuAssisted { false },
        .gpuAssistedReserveBindingSlot { false },
        .bestPractices { false },
        .debugPrintf { false },
        .synchronizationValidation { false },
        .disableAll { false },
        .disableShaders { false },
        .disableThreadSafety { false },
        .disableAPIParameters { false },
        .disableObjectLifetimes { false },
        .disableCoreChecks { false },
        .disableUniqueHandles { false },
        .disableShaderValidationCache { false },
    };
    Instance::ExtensionSettings requiredExtensions
    {
        .surface {1},
        .win32Surface {1},
        //.validationFeatures = 1,
        .debugUtils = 1,
    };
    Instance::ExtensionSettings optionalExtensions{
        .swapchainColorSpaceExtension {1}
    };
    auto applicationName = "HeadlessSample";
    auto engineName = "NYAN";
    auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
    if (!instanceCreateResult)
        return false;
    instance = std::make_unique<Instance>(std::move(*instanceCreateResult));

    PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions
    {
        .swapchain{1}
    };

    auto physicalDeviceResult = instance->select_physical_device(std::nullopt, requiredDeviceExtensions);

    if (!physicalDeviceResult)
        return false;

    auto physicalDevice = *physicalDeviceResult;

    QueueContainer<float> queuePriorities;
    nyan::ignore = queuePriorities[Queue::Type::Graphics].push_back(1.f);
    //nyan::ignore = queuePriorities[Queue::Type::Compute].push_back(1.f);
    //nyan::ignore = queuePriorities[Queue::Type::Transfer].push_back(1.f);

    //std::cout << "Supports Rebar: " << physicalDevice.supports_rebar() << std::endl;

    auto logicalDeviceCreateResult = LogicalDevice::create(*instance, physicalDevice,
        nullptr, requiredDeviceExtensions, queuePriorities);
    if (!logicalDeviceCreateResult)
        return false;
    logicalDevice = std::make_unique<LogicalDevice>(std::move(*logicalDeviceCreateResult));


    if (logicalDevice->get_queues(Queue::Type::Graphics).size() != queuePriorities[Queue::Type::Graphics].size())
        return false;

    if (logicalDevice->get_queues(Queue::Type::Compute).size() != queuePriorities[Queue::Type::Compute].size())
        return false;

    if (logicalDevice->get_queues(Queue::Type::Transfer).size() != queuePriorities[Queue::Type::Transfer].size())
        return false;

    auto graphicsQueues = logicalDevice->get_queues(Queue::Type::Graphics);
    {
        assert(graphicsQueues[0].wait_idle());
    }

    auto nativeResult = window.get_platform_data();
    if (!nativeResult)
        return false;

    auto surfaceResult = Surface::create(*instance, nativeResult->hwnd, nativeResult->hinstance);
    if (!surfaceResult) {
        log::error_message("couldn't create surface");
        return false;
    }

    Format swapchainFormat;
    ColorSpace swapchainColorSpace;
    {
        auto surfaceFormatsRes = surfaceResult->get_surface_formats(logicalDevice->get_physical_device());    
        if (!surfaceFormatsRes) {
            log::error_message("couldn't create surface");
            return false;
        }
        auto surfaceFormats = *surfaceFormatsRes;
        swapchainFormat = surfaceFormats[0].format;
        swapchainColorSpace = surfaceFormats[0].colorSpace;
    }

    auto swapchainResult = Swapchain::create(*logicalDevice, *surfaceResult, Swapchain::Params{ .format {swapchainFormat }, .colorSpace{swapchainColorSpace } });

    if (!swapchainResult) {
        log::error_message("couldn't create swapchain");
        return false;
    }


    //auto setLayoutResult = DescriptorSetLayout::create(*logicalDevice,
    //    DescriptorSetLayout::DescriptorInfo{
    //        .storageBufferCount{ 0 },
    //        .uniformBufferCount{ 0 },
    //        .samplerCount{ 0 },
    //        .sampledImageCount{ 0 },
    //        .storageImageCount{ 0 },
    //        .inputAttachmentCount{ 0 },
    //        .accelerationStructureCount{ 0 }
    //    });

    //if (!setLayoutResult)
    //    return false;

    //auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

    //if (!setPoolResult)
    //    return false;

    //auto& sets = setPoolResult->get_sets();

    auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { });

    if (!pipelineLayoutResult)
        return false;

    //static constexpr uint32_t rtWidth = 4;
    //static constexpr uint32_t rtHeight = 4;
    //static constexpr Samples rtSamples = Samples::C1;
    //static constexpr RenderFormat rtFormat = RenderFormat::B8G8R8A8_UNORM;

    //auto rtResult = RenderTarget::create(*logicalDevice, RenderTarget::RenderTargetOptions{
    //        .options{
    //            .queueFamilies {},
    //            .usage {ImageUsageFlags::ColorAttachment, ImageUsageFlags::TransferSrc}
    //        },
    //        .width{ rtWidth },
    //        .height{ rtHeight },
    //        .samples {rtSamples},
    //        .format{ rtFormat},
    //        .dedicatedAllocation{ false }
    //    });

    //if (!rtResult)
    //    return false;

    auto renderPassResult = SingleRenderPass::create(*logicalDevice,
        SingleRenderPass::Params{ .attachments { SingleRenderPass::Attachment{
            .load{ LoadOp::Clear },
            .store{ StoreOp::Store },
            .format{static_cast<Format>(swapchainFormat)},
            .samples{ Samples::C1 },
            .initialLayout{ Layout::Undefined },
            .finalLayout{ Layout::PresentSrc },
            .clearValue{1.f, 0.f, 0.f, 1.f}
        }} });

    if (!renderPassResult) {
        log::error_message("couldn't create Renderpass");
        return false;
    }

    nyan::StaticVector<Framebuffer, 4> framebuffers;

    for (auto& image : swapchainResult->get_images()) {
        auto framebufferResult = Framebuffer::create(*logicalDevice, { .renderPass {*renderPassResult}, .attachments{image.get_image_view(0)}, .width {static_cast<uint16_t>(swapchainResult->get_width())},. height {static_cast<uint16_t>(swapchainResult->get_height())}});

        if (!framebufferResult) {
            log::error_message("couldn't create framebuffer");
            return false;
        }
        nyan::ignore = framebuffers.push_back(std::move(*framebufferResult));
    }

    auto vertShaderCode = read_binary_file("shader/triangle_test_vert.spv");
    if (vertShaderCode.empty()) {
        log::error_message("couldn't read vert shader");
        return false;
    }

    auto vertShaderResult = Shader::create(*logicalDevice, vertShaderCode);
    if (!vertShaderResult)
        return false;
    //ShaderCode can be freed now

    auto vertShaderInstance = vertShaderResult->create_shader_instance();


    auto fragShaderCode = read_binary_file("shader/triangle_test_frag.spv");
    if (fragShaderCode.empty()) {
        log::error_message("couldn't read frag shader");
        return false;
    }

    auto fragShaderResult = Shader::create(*logicalDevice, fragShaderCode);
    if (!fragShaderResult)
        return false;
    //ShaderCode can be freed now

    auto fragShaderInstance = fragShaderResult->create_shader_instance();

    auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
    if (!pipelineCacheResult)
        return false;

    const GraphicsPipeline::PipelineState defaultPipelineState{
        .dynamicState{},
        .stencilFrontCompareMask { 0 },
        .stencilFrontWriteMask { 0 },
        .stencilFrontReference { 0 },
        .stencilBackCompareMask { 0 },
        .stencilBackWriteMask { 0 },
        .stencilBackReference { 0 },
        .depthWriteEnable {0},
        .depthTestEnable {0},
        .depthBiasEnable {0},
        .depthBoundsTestEnable {0},
        .depthClampEnable {0},
        .stencilTestEnable {0},
        .primitiveRestartEnable {0},
        .rasterizerDiscardEnable {0},
        .depthCompareOp {0},
        .stencilFrontFailOp { 0 },
        .stencilFrontPassOp { 0 },
        .stencilFrontDepthFailOp { 0 },
        .stencilFrontCompareOp { 0 },
        .stencilBackFailOp { 0 },
        .stencilBackPassOp { 0 },
        .stencilBackDepthFailOp { 0 },
        .stencilBackCompareOp { 0 },
        .cullMode { 0 },
        .frontFace {1},
        .primitiveTopology {3},
        .polygonMode { 0 },
        .rasterizationSampleCount { 1 },
        .alphaToCoverageEnable {0},
        .alphaToOneEnable {0},
        .sampleShadingEnable {0},
        .logicOpEnable {0},
        .logicOp { 0 },
        .patchControlPoints { 0},
        .sampleMask {15},
        .minSampleShading{ 0.f },
        .depthBiasConstantFactor{ 0.f },
        .depthBiasClamp{ 0.f },
        .depthBiasSlopeFactor{ 0.f },
        .lineWidth{ 1.f },
        .minDepthBounds{ 0.f },
        .maxDepthBounds{ 1.f },
        .blendConstants{ 0.f, 0.f, 0.f, 0.f },
        .viewport{
            .width{ static_cast<uint16_t>(swapchainResult->get_width()) },
            .height{ static_cast<uint16_t>(swapchainResult->get_height()) },
        },
        .blendAttachments{GraphicsPipeline::BlendAttachment {
            .colorWriteMask {15}
        }},
    };
    VertexShaderGraphicsPipeline::Parameters pipelineParams{
        .vertexInput{},
        .pipelineState{defaultPipelineState},
        .renderInfo{GraphicsPipeline::RenderPassInfo{*renderPassResult, 0}},
        .vertexShader{vertShaderInstance},
        .fragmentShader{fragShaderInstance}
    };

    auto pipelineResult = VertexShaderGraphicsPipeline::create(*logicalDevice, *pipelineLayoutResult, pipelineParams, &*pipelineCacheResult);

    if (!pipelineResult) {
        log::error_message("couldn't create pipeline");
        return false;
    }

    //ShaderModule, PipelineCache and PipelineLayout could be destroyed now

    auto commandPoolResult = CommandPool::create(*logicalDevice, graphicsQueues[0]);
    if (!commandPoolResult)
        return false;

    auto commandBufferResult = commandPoolResult->allocate_command_buffer();
    constexpr auto testSize = 1;

    if (!commandBufferResult)
        return false;


    uint32_t counter = 0;
    auto start = std::chrono::steady_clock::now();
    for (; !window.should_close();) {
        //if (i != 0)
        if(auto result = swapchainResult->aquire_image(); !result) {
            log::error_message("couldn't acquire");
            return false;
        }

        if (auto result = commandPoolResult->reset(); !result) {
            log::error_message("couldn't reset");
            return false;
        }

        if (auto beginResult = commandBufferResult->begin(); !beginResult) {
            log::error_message("couldn't begin");
            return false;
        }
        auto index = swapchainResult->get_index();

        auto bind = commandBufferResult->bind_pipeline(*pipelineResult);
        bind.begin_render_pass(*renderPassResult, framebuffers[index]);
        bind.draw(3);
        bind.end_render_pass();
        commandBufferResult->barrier2();
        if (auto endResult = commandBufferResult->end(); !endResult) {
            log::error_message("couldn't end");
            return false;
        }
        if (auto submitResult = graphicsQueues[0].submit(*commandBufferResult, swapchainResult->get_acquire_semaphores()[index], swapchainResult->get_present_semaphores()[index]); !submitResult) {
            log::error_message("couldn't submit");
            return false;
        }

        if (auto result = graphicsQueues[0].present(*swapchainResult, &swapchainResult->get_present_semaphores()[index]); !result) {
            log::error_message("couldn't present");
            return false;
        }
        if (!graphicsQueues[0].wait_idle()) {
            log::error_message("couldn't wait idle");
            return false;
        }
        //if (!logicalDevice->wait_idle()) {
        //    log::error_message("couldn't wait idle");
        //    return false;
        //}
        library.poll_events();
        counter++;
        if (!(counter & 1023)) {
            auto end = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration<double, std::milli>{ end - start }.count();
            log::verbose_message("1024 frames passed").format(" in {}ms => {} fps", ms, std::milli::den * 1024.f/ ms);
            start = end;
        }

    }
    return true;
}

[[nodiscard]] ExitCode better_main([[maybe_unused]] std::span<const std::string_view> args) {
    auto name = "NYAN: Minimal Swapchain Sample";

    //if (!glfwInit())
    //    return ExitCode::Failure;
    //if(!glfwVulkanSupported())
    //    return ExitCode::Failure;


    constexpr uint32_t windowWidth = 1920;
    constexpr uint32_t windowHeight = 1080;

    auto libResult = nyan::glfw::Library::create();
    if (!libResult)
        return ExitCode::Failure;
    auto library = std::move(*libResult);

    if (!library.vulkan_supported())
        return ExitCode::Failure;

    auto winResult = library.create_window({ .width{windowWidth},.height{windowHeight}, .windowTitle{name} });
    if(!winResult)
        return ExitCode::Failure;

    if (!setup(*winResult, library)) {
        logicalDevice.release();
        instance.release();
        return ExitCode::Failure;
    }

    logicalDevice.release();
    instance.release();

    return ExitCode::Success;
}