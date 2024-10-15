import std;
import NYANVulkan;
import NYANAssert;
//
//#include <cassert>
//#include <fstream>
//#include <filesystem>
//#include <vector>
//#include <array>
//#include <optional>
//#include <string_view>
//#include <span>
//#include <string>

using namespace nyan;
using namespace nyan::vulkan;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

enum class ExitCode : int {
	Success = 0,
	Failure = 1
};

inline static std::unique_ptr<Instance> instance{ nullptr };
//inline static std::unique_ptr<LogicalDevice> logicalDevice{ nullptr };

constexpr inline auto Lassert = nyan::assert::Assert{};

static std::vector<uint32_t> read_binary_file(const std::filesystem::path& path)
{

    std::vector<uint32_t> data;
    std::ifstream in(path, std::ios::binary);
    if (in.is_open()) {
        auto size = std::filesystem::file_size(path);
        data.resize(size / sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(data.data()), size);
        in.close();
    }
    else {
    }

    return data;
}
bool setup() {
    Instance::ValidationSettings validationSettings{
        .enabled {true},
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
        .debugPrintf { true },
        .synchronizationValidation { true },
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
        .validationFeatures = 1,
        .debugUtils = 1,
    };
    Instance::ExtensionSettings optionalExtensions{
    };
    auto applicationName = "HeadlessSample";
    auto engineName = "NYAN";
    auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
    if (!instanceCreateResult)
        return false;
    instance = std::make_unique<Instance>(std::move(*instanceCreateResult));

    PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions
    {
        .pushDescriptors {1}
    };

    auto physicalDeviceResult = instance->select_physical_device(std::nullopt, requiredDeviceExtensions);

    if (!physicalDeviceResult)
        return false;

    auto physicalDevice = *physicalDeviceResult;

    QueueContainer<float> queuePriorities;
    nyan::ignore = queuePriorities[Queue::Type::Graphics].push_back(1.f);
    nyan::ignore = queuePriorities[Queue::Type::Compute].push_back(1.f);
    nyan::ignore = queuePriorities[Queue::Type::Transfer].push_back(1.f);

    //std::cout << "Supports Rebar: " << physicalDevice.supports_rebar() << std::endl;

    auto logicalDevice = LogicalDevice::create(*instance, physicalDevice,
        nullptr, requiredDeviceExtensions, queuePriorities);
    if (!logicalDevice)
        return false;
    //logicalDevice = std::make_unique<LogicalDevice>(std::move(*logicalDeviceCreateResult));


    if (logicalDevice->get_queues(Queue::Type::Graphics).size() != queuePriorities[Queue::Type::Graphics].size())
        return false;

    if (logicalDevice->get_queues(Queue::Type::Compute).size() != queuePriorities[Queue::Type::Compute].size())
        return false;

    if (logicalDevice->get_queues(Queue::Type::Transfer).size() != queuePriorities[Queue::Type::Transfer].size())
        return false;

    auto graphicsQueues = logicalDevice->get_queues(Queue::Type::Graphics);
    {
        ::Lassert(!!graphicsQueues[0].wait_idle());
    }

    auto setLayoutResult = DescriptorSetLayout::create(*logicalDevice,
        DescriptorSetLayout::DescriptorInfo{
            .storageBufferCount{ 2 },
            .uniformBufferCount{ 0 },
            .samplerCount{ 0 },
            .sampledImageCount{ 0 },
            .storageImageCount{ 0 },
            .inputAttachmentCount{ 0 },
            .accelerationStructureCount{ 0 }
        });

    if (!setLayoutResult)
        return false;

    auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

    if (!setPoolResult)
        return false;

    auto& sets = setPoolResult->get_sets();


    auto pushSetLayoutResult = PushDescriptorSetLayout::create(*logicalDevice, DescriptorSetLayout::DescriptorInfo{
            .storageBufferCount{ 2 },
            .uniformBufferCount{ 0 },
            .samplerCount{ 0 },
            .sampledImageCount{ 0 },
            .storageImageCount{ 0 },
            .inputAttachmentCount{ 1 },
            .accelerationStructureCount{ 0 }
        });

    if (!pushSetLayoutResult)
        return false;


    auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { static_cast<DescriptorSetLayout*>(&*setLayoutResult), 1 });

    if (!pipelineLayoutResult)
        return false;

    static constexpr uint32_t rtWidth = 4;
    static constexpr uint32_t rtHeight = 4;
    static constexpr Samples rtSamples = Samples::C1;
    static constexpr RenderFormat rtFormat = RenderFormat::B8G8R8A8_UNORM;

    auto rtResult = RenderTarget::create(*logicalDevice, RenderTarget::RenderTargetOptions{
            .options{
                .queueFamilies {},
                .usage {ImageUsageFlags::ColorAttachment, ImageUsageFlags::TransferSrc},

            },
            .width{ rtWidth },
            .height{ rtHeight },
            .samples {rtSamples},
            .format{ rtFormat}
        });

    if (!rtResult)
        return false;

    auto renderPassResult = SingleRenderPass::create(*logicalDevice,
        SingleRenderPass::Params{ .attachments { SingleRenderPass::Attachment{
            .load{ LoadOp::Clear },
            .store{ StoreOp::Store },
            .format{static_cast<Format>(rtFormat)},
            .samples{ rtSamples },
            .initialLayout{ Layout::Undefined },
            .finalLayout{ Layout::TransferSrc },
            .clearValue{0x00FFFF00u}
        }} });

    if (!renderPassResult)
        return false;

    auto framebufferResult = Framebuffer::create(*logicalDevice, { .renderPass {*renderPassResult}, .attachments{rtResult->get_image_view(0)}, .width {rtWidth},. height {rtHeight} });

    if (!framebufferResult)
        return false;

    auto vertShaderCode = read_binary_file("shader/triangle_test_vert.spv");
    if (vertShaderCode.empty())
        return false;

    auto vertShaderResult = Shader::create(*logicalDevice, vertShaderCode);
    if (!vertShaderResult)
        return false;
    //ShaderCode can be freed now

    auto vertShaderInstance = vertShaderResult->create_shader_instance();


    auto fragShaderCode = read_binary_file("shader/triangle_test_frag.spv");
    if (fragShaderCode.empty())
        return false;

    auto fragShaderResult = Shader::create(*logicalDevice, fragShaderCode);
    if (!fragShaderResult)
        return false;
    //ShaderCode can be freed now

    auto fragShaderInstance = fragShaderResult->create_shader_instance();

    auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
    if (!pipelineCacheResult)
        return false;

    static constexpr GraphicsPipeline::PipelineState defaultPipelineState{
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
        .sampleMask { std::numeric_limits<uint32_t>::max() },
        .minSampleShading{ 0.f },
        .depthBiasConstantFactor{ 0.f },
        .depthBiasClamp{ 0.f },
        .depthBiasSlopeFactor{ 0.f },
        .lineWidth{ 1.f },
        .minDepthBounds{ 0.f },
        .maxDepthBounds{ 1.f },
        .blendConstants{ 0.f, 0.f, 0.f, 0.f },
        .viewport{
            .width{ rtWidth },
            .height{ rtHeight },
        },
        .blendAttachments{GraphicsPipeline::BlendAttachment {
             .blendEnable {0},
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

    if (!pipelineResult)
        return false;

    //ShaderModule, PipelineCache and PipelineLayout could be destroyed now

    auto commandPoolResult = CommandPool::create(*logicalDevice, graphicsQueues[0]);
    if (!commandPoolResult)
        return false;

    auto commandBufferResult = commandPoolResult->allocate_command_buffer();
    constexpr auto testSize = 1;

    if (!commandBufferResult)
        return false;

    constexpr size_t testBuffSize = format_byte_size(rtFormat) * rtWidth * rtHeight;

    auto bufferResult = StorageBuffer::create(*logicalDevice, Buffer::Options{ .size {testBuffSize}, .dedicatedAllocation = false, .mapable = true });

    if (!bufferResult)
        return false;

    uint32_t counter = 0;
    for (auto i = 0; i < testSize; ++i) {
        if (i != 0)
            if (auto result = commandPoolResult->reset(); !result)
                return false;

        if (auto beginResult = commandBufferResult->begin(); !beginResult)
            return false;


        auto bind = commandBufferResult->bind_pipeline(*pipelineResult);
        bind.begin_render_pass(*renderPassResult, *framebufferResult);
        bind.draw(3);
        bind.end_render_pass();
        commandBufferResult->barrier2();
        if (auto endResult = commandBufferResult->end(); !endResult)
            return false;
        if (auto submitResult = graphicsQueues[0].submit2(*commandBufferResult); !submitResult)
            return false;
        if (!graphicsQueues[0].wait_idle())
            return false;
        if (auto result = commandPoolResult->reset(); !result)
            return false;

        if (auto beginResult = commandBufferResult->begin(); !beginResult)
            return false;

        commandBufferResult->barrier2();
        commandBufferResult->copy_image_to_buffer(*rtResult, Layout::TransferSrc, *bufferResult);
        commandBufferResult->barrier2();

        if (auto endResult = commandBufferResult->end(); !endResult)
            return false;

        if (auto submitResult = graphicsQueues[0].submit2(*commandBufferResult); !submitResult)
            return false;

        if (!graphicsQueues[0].wait_idle())
            return false;
        if (!logicalDevice->wait_idle())
            return false;

        ::Lassert(!!bufferResult->flush());
        ::Lassert(!!bufferResult->invalidate());
        auto ptr = *reinterpret_cast<std::array<uint8_t, testBuffSize>*>(bufferResult->mapped_data());

        if (ptr[0] == 255 && ptr[3] == 255)
            counter++;
    }
    return true;
}
//bool loop() {
//
//}

int main(const int argc, char const* const* const argv)
{
	[[nodiscard]] ExitCode better_main(std::span<const std::string_view>);
	std::vector<std::string_view>
		args(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));

	return static_cast<int>(better_main(args));
}


[[nodiscard]] ExitCode better_main([[maybe_unused]] std::span<const std::string_view> args) {
	auto name = "Headless";

    if (!setup()) {
        //logicalDevice.release();
        instance.release();
        return ExitCode::Failure;
    }

    //logicalDevice.release();
    instance.release();

	return ExitCode::Success;
}