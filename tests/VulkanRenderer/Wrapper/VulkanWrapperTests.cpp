#include <gtest/gtest.h>
import std;
import NYANVulkan;
import NYANLog;
//
//#include <memory>
//#include <array>
//#include <vector>
//#include <fstream>
//#include <filesystem>


//#define NOMINMAX
//#include <Windows.h>
//
//static uint64_t GetAvailableStackSpace()
//{
//    volatile uint8_t var;
//    MEMORY_BASIC_INFORMATION mbi;
//
//    auto virtualQuerySuccess = VirtualQuery((LPCVOID)&var, &mbi, sizeof(mbi));
//
//    if (!virtualQuerySuccess)
//    {
//        return 0;
//    }
//
//    return &var - mbi.AllocationBase;
//}
//
//static size_t GetStackUsage()
//{
//    MEMORY_BASIC_INFORMATION mbi;
//    VirtualQuery(&mbi, &mbi, sizeof(mbi));
//    // now mbi.AllocationBase = reserved stack memory base address
//
//    VirtualQuery(mbi.AllocationBase, &mbi, sizeof(mbi));
//    // now (mbi.BaseAddress, mbi.RegionSize) describe reserved (uncommitted) portion of the stack
//    // skip it
//
//    VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
//    // now (mbi.BaseAddress, mbi.RegionSize) describe the guard page
//    // skip it
//
//    VirtualQuery((char*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(mbi));
//    // now (mbi.BaseAddress, mbi.RegionSize) describe the committed (i.e. accessed) portion of the stack
//
//    return mbi.RegionSize;
//}
//
//auto stackSize = GetAvailableStackSpace();
//auto usedStack = GetStackUsage();

namespace nyan
{
	namespace vulkan
	{
        namespace wrapper {
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
            auto instance_test_setup()
            {
                Instance::ValidationSettings validationSettings{};
                Instance::ExtensionSettings requiredExtensions{};
                Instance::ExtensionSettings optionalExtensions{};
                auto applicationName = "InstanceTests";
                auto engineName = "InstanceTests";
                return Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
            }
            TEST(InstanceTest, Setup)
            {
                auto instance = instance_test_setup();
                EXPECT_TRUE(instance);
            }

            TEST(InstanceTest, Validation)
            {
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
                    .validationFeatures = 1,
                    .debugUtils = 1,
                };
                Instance::ExtensionSettings optionalExtensions{};
                auto applicationName = "InstanceTests";
                auto engineName = "InstanceTests";
                {
                    auto instance = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
                    EXPECT_TRUE(instance);
                }
            }
            struct InstanceTestFixture : public ::testing::Test {
                static void SetUpTestSuite()
                {
                    auto instanceCreateResult = instance_test_setup();
                    if (!instanceCreateResult)
                        GTEST_SKIP() << "Could not create Instance, skipping Instance Tests";

                    instance = std::make_unique<Instance>(std::move(*instanceCreateResult));
                }
                InstanceTestFixture()
                {

                }
                void SetUp() override
                {
                }
                //--Test-Body--
                //void TearDown();
                //~InstanceTestFixture()
                //--end for each
                static void TearDownTestSuite()
                {
                    instance.release();
                }
                inline static std::unique_ptr<Instance> instance{ nullptr };
            };
            TEST_F(InstanceTestFixture, a)
            {

            }
            struct LogicalDeviceTestFixture : public ::testing::Test {
                static void SetUpTestSuite()
                {
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
                        .validationFeatures = 1,
                        .debugUtils = 1,
                    };
                    Instance::ExtensionSettings optionalExtensions{
                    };
                    auto applicationName = " LogicalDeviceTests";
                    auto engineName = " LogicalDeviceTests";
                    auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
                    if (!instanceCreateResult)
                        GTEST_SKIP() << "Could not create Instance, skipping  Logical Device Tests";
                    instance = std::make_unique<Instance>(std::move(*instanceCreateResult));

                    PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions
                    {
                        .pushDescriptors {1}
                    };

                    auto physicalDeviceResult = instance->select_physical_device(std::nullopt, requiredDeviceExtensions);

                    if (!physicalDeviceResult)
                        GTEST_SKIP() << "Could not select physical Device, skipping Logical Device Tests" << physicalDeviceResult.error().get_message();

                    auto physicalDevice = *physicalDeviceResult;

                    QueueContainer<float> queuePriorities;
                    nyan::ignore = queuePriorities[Queue::Type::Graphics].push_back(1.f);
                    nyan::ignore = queuePriorities[Queue::Type::Compute].push_back(1.f);
                    nyan::ignore = queuePriorities[Queue::Type::Transfer].push_back(1.f);

                    //std::cout << "Supports Rebar: " << physicalDevice.supports_rebar() << std::endl;

                    auto logicalDeviceCreateResult = LogicalDevice::create(*instance, physicalDevice,
                        nullptr, requiredDeviceExtensions, queuePriorities);
                    if (!logicalDeviceCreateResult)
                        GTEST_SKIP() << "Could not create LogicalDevice, skipping Logical Device Tests";
                    logicalDevice = std::make_unique<LogicalDevice>(std::move(*logicalDeviceCreateResult));


                    if (logicalDevice->get_queues(Queue::Type::Graphics).size() != queuePriorities[Queue::Type::Graphics].size())
                        GTEST_SKIP() << "Error creating Graphics queue, skipping Logical Device Tests";

                    //if (logicalDevice->get_queues(Queue::Type::Compute).size() != queuePriorities[Queue::Type::Compute].size())
                    //    GTEST_SKIP() << "Error creating Compute queue, skipping Logical Device Tests";

                    //if (logicalDevice->get_queues(Queue::Type::Transfer).size() != queuePriorities[Queue::Type::Transfer].size())
                    //    GTEST_SKIP() << "Error creating Transfer queue, skipping Logical Device Tests";

                }
                LogicalDeviceTestFixture()
                {

                }
                void SetUp() override
                {
                }
                //--Test-Body--
                //void TearDown();
                //~InstanceTestFixture()
                //--end for each
                static void TearDownTestSuite()
                {
                    //Mandatory since dlls will be unloaded before these static variables are deleted
                    logicalDevice.release();
                    instance.release();
                }
                inline static std::unique_ptr<Instance> instance{ nullptr };
                inline static std::unique_ptr<LogicalDevice> logicalDevice{ nullptr };
            };
            TEST_F(LogicalDeviceTestFixture, MinimalComputeTest)
            {

                auto graphicsQueues = logicalDevice->get_queues(Queue::Type::Graphics);
                {
                    if(!graphicsQueues[0].wait_idle())
                        GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";
                }
                //auto computeQueues = logicalDevice->get_queues(Queue::Type::Compute);
                //{
                //    GTEST_ASSERT_TRUE(computeQueues[0].wait_idle()) << "Error waiting on Compute queue, skipping Logical Device Tests";
                //}
                //auto transferQueues = logicalDevice->get_queues(Queue::Type::Transfer);
                //{
                //    GTEST_ASSERT_TRUE(transferQueues[0].wait_idle()) << "Error waiting on Transfer queue, skipping Logical Device Tests";
                //}

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
                    GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";

                auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

                if (!setPoolResult)
                    GTEST_SKIP() << "Error creating descriptor set pool, skipping Logical Device Tests";

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
                    GTEST_SKIP() << "Error creating push descriptor set layout, skipping Logical Device Tests";

                auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { static_cast<DescriptorSetLayout*>(&*setLayoutResult), 1 });

                if (!pipelineLayoutResult)
                    GTEST_SKIP() << "Error creating pipelineLayout, skipping Logical Device Tests";

                auto shaderCode = read_binary_file("shader/test_comp.spv");
                if (shaderCode.empty())
                    GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                auto shaderResult = Shader::create(*logicalDevice, shaderCode);
                if (!shaderResult)
                    GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";
                //ShaderCode can be freed now
                constexpr uint32_t testSize = 4;

                auto shaderInstance = shaderResult->create_shader_instance();
                GTEST_EXPECT_TRUE(shaderInstance.set_spec_constant(1, 1u));
                GTEST_EXPECT_TRUE(shaderInstance.set_spec_constant(2, 1u));
                GTEST_EXPECT_TRUE(shaderInstance.set_spec_constant(3, 1u));
                GTEST_EXPECT_TRUE(shaderInstance.set_spec_constant("testVal", testSize));


                auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
                if (!pipelineCacheResult)
                    GTEST_SKIP() << "Error creating pipeline Cache, skipping Logical Device Tests";

                auto pipelineResult = ComputePipeline::create(logicalDevice->get_device(), *pipelineLayoutResult, shaderInstance, &*pipelineCacheResult);

                if (!pipelineResult)
                    GTEST_SKIP() << "Error creating pipeline, skipping Logical Device Tests";
                //ShaderModule, PipelineCache and PipelineLayout could be destroyed now

                auto bufferResult = StorageBuffer::create(*logicalDevice, Buffer::Options{ .size {sizeof(uint32_t) * testSize * 2}, .dedicatedAllocation = true, .mapable = true});

                if (!bufferResult)
                    GTEST_SKIP() << "Error creating buffer, skipping Logical Device Tests " << bufferResult.error().get_message();

                auto descriptorResultA = sets[0].add(*bufferResult, 0, sizeof(uint32_t) * testSize);

                if (!descriptorResultA)
                    GTEST_SKIP() << "Error updating DescriptorSet, skipping Logical Device Tests " << descriptorResultA.error().get_message();

                auto descriptorResultB = sets[0].add(*bufferResult, sizeof(uint32_t) * testSize, sizeof(uint32_t) * testSize);

                if (!descriptorResultB)
                    GTEST_SKIP() << "Error updating DescriptorSet, skipping Logical Device Tests " << descriptorResultB.error().get_message();

                auto* inptr = reinterpret_cast<std::array<uint32_t, testSize>*>(bufferResult->mapped_data());
                auto* outptr = reinterpret_cast<std::array<uint32_t, testSize>*>(bufferResult->mapped_data()) + 1;
                if (inptr == nullptr)
                    GTEST_SKIP() << "Mapped Buffer Data not available";

                (*inptr)[0] = 5;
                (*inptr)[1] = 6;
                (*inptr)[2] = 7;
                (*inptr)[3] = 8;

                (*outptr)[0] = 1;
                (*outptr)[1] = 1;
                (*outptr)[2] = 1;
                (*outptr)[3] = 1;

                if (!bufferResult->flush())
                    GTEST_SKIP() << "Error flushing Buffer, skipping Logical Device Tests " << bufferResult.error().get_message();

                auto commandPoolResult = CommandPool::create(*logicalDevice, graphicsQueues[0]);
                if (!commandPoolResult)
                    GTEST_SKIP() << "Error creating Command Pool, skipping Logical Device Tests " << commandPoolResult.error().get_message();

                auto commandBufferResult = commandPoolResult->allocate_command_buffer();

                if (!commandBufferResult)
                    GTEST_SKIP() << "Error allocating Command Buffer, skipping Logical Device Tests " << commandBufferResult.error().get_message();

                auto beginResult = commandBufferResult->begin();
                if (!beginResult)
                    GTEST_SKIP() << "Error beginning Command Buffer, skipping Logical Device Tests " << beginResult.error().get_message();

                auto bind = commandBufferResult->bind_pipeline(*pipelineResult);
                bind.bind_descriptor_set(0, sets[0]);

                struct PushConstants {
                    uint32_t binding0;
                    uint32_t binding1;
                } constants{ descriptorResultA->value, descriptorResultB->value };
                bind.push_constants(constants);
                bind.dispatch(1, 1, 1);

                auto endResult = commandBufferResult->end();
                if (!endResult)
                    GTEST_SKIP() << "Error ending Command Buffer, skipping Logical Device Tests " << endResult.error().get_message();

                auto submitResult = graphicsQueues[0].submit2(*commandBufferResult);

                if (!submitResult)
                    GTEST_SKIP() << "Error submiting Command Buffer, skipping Logical Device Tests " << submitResult.error().get_message();

                if (!graphicsQueues[0].wait_idle())
                    GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";

                GTEST_ASSERT_TRUE(bufferResult->invalidate());
                bool result = true;

                for (int i = 0; i < testSize; ++i)
                    GTEST_ASSERT_EQ((*inptr)[i] + testSize, (*outptr)[i] ) << "Invalid shader results, skipping Logical Device Tests";

            }
            TEST_F(LogicalDeviceTestFixture, MinimalGraphicsRenderingTests)
            {
                auto graphicsQueues = logicalDevice->get_queues(Queue::Type::Graphics);
                {
                    GTEST_ASSERT_TRUE(graphicsQueues[0].wait_idle()) << "Error waiting on Graphics queue, skipping Logical Device Tests";
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
                    GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";

                auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

                if (!setPoolResult)
                    GTEST_SKIP() << "Error creating descriptor set pool, skipping Logical Device Tests";

                auto& sets = setPoolResult->get_sets();

                auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { static_cast<DescriptorSetLayout*>(&*setLayoutResult), 1 });

                if (!pipelineLayoutResult)
                    GTEST_SKIP() << "Error creating pipelineLayout, skipping Logical Device Tests";

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
                        .format{ rtFormat},
                        .dedicatedAllocation{ true },
                    });

                if (!rtResult)
                    GTEST_SKIP() << "Error creating Render Target, skipping Logical Device Tests " << rtResult.error().get_message();

                auto vertShaderCode = read_binary_file("shader/triangle_test_vert.spv");
                if (vertShaderCode.empty())
                    GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                auto vertShaderResult = Shader::create(*logicalDevice, vertShaderCode);
                if (!vertShaderResult)
                    GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";
                //ShaderCode can be freed now

                auto vertShaderInstance = vertShaderResult->create_shader_instance();


                auto fragShaderCode = read_binary_file("shader/triangle_test_frag.spv");
                if (fragShaderCode.empty())
                    GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                auto fragShaderResult = Shader::create(*logicalDevice, fragShaderCode);
                if (!fragShaderResult)
                    GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";
                //ShaderCode can be freed now

                auto fragShaderInstance = fragShaderResult->create_shader_instance();

                auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
                if (!pipelineCacheResult)
                    GTEST_SKIP() << "Error creating pipeline Cache, skipping Logical Device Tests";

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
                    .renderInfo{GraphicsPipeline::RenderingCreateInfo {
                        .viewMask{ 0 },
                        .colorAttachmentFormats { rtFormat },
                        .colorAttachmentCount{ 1 },
                    }},
                    .vertexShader{vertShaderInstance},
                    .fragmentShader{fragShaderInstance}
                };

                auto pipelineResult = VertexShaderGraphicsPipeline::create(*logicalDevice, *pipelineLayoutResult, pipelineParams, nullptr);

                if (!pipelineResult)
                    GTEST_SKIP() << "Error creating pipeline, skipping Logical Device Tests";

                //ShaderModule, PipelineCache and PipelineLayout could be destroyed now

                auto commandPoolResult = CommandPool::create(*logicalDevice, graphicsQueues[0]);
                if (!commandPoolResult)
                    GTEST_SKIP() << "Error creating Command Pool, skipping Logical Device Tests " << commandPoolResult.error().get_message();

                auto commandBufferResult = commandPoolResult->allocate_command_buffer();
                constexpr auto testSize = 1;

                if (!commandBufferResult)
                    GTEST_SKIP() << "Error allocating Command Buffer, skipping Logical Device Tests " << commandBufferResult.error().get_message();

                constexpr size_t testBuffSize = format_byte_size(rtFormat) * rtWidth * rtHeight;

                auto bufferResult = StorageBuffer::create(*logicalDevice, Buffer::Options{ .size {testBuffSize}, .dedicatedAllocation = true, .mapable = true });

                if (!bufferResult)
                    GTEST_SKIP() << "Error creating buffer, skipping Logical Device Tests " << bufferResult.error().get_message();

                uint32_t counter = 0;
                for (auto i = 0; i < testSize; ++i) {
                    if (i != 0)
                        if (auto result = commandPoolResult->reset(); !result)
                            GTEST_SKIP() << "Error reseting Command Pool, skipping Logical Device Tests";

                    if (auto beginResult = commandBufferResult->begin(); !beginResult)
                        GTEST_SKIP() << "Error beginning Command Buffer, skipping Logical Device Tests " << beginResult.error().get_message();

                    RenderingInfo renderingInfo{ .colorAttachments {RenderingInfo::Attachment{.image{*rtResult}}} };

                    commandBufferResult->image_barrier2(*rtResult, Layout::Undefined, Layout::ColorAttachment);

                    auto bind = commandBufferResult->bind_pipeline(*pipelineResult);
                    bind.begin_rendering(renderingInfo);
                    bind.draw(3);
                    bind.end_rendering();
                    commandBufferResult->image_barrier2(*rtResult, Layout::ColorAttachment, Layout::TransferSrc );
                    commandBufferResult->copy_image_to_buffer(*rtResult, Layout::TransferSrc, *bufferResult);
                    commandBufferResult->barrier2();

                    if (auto endResult = commandBufferResult->end(); !endResult)
                        GTEST_SKIP() << "Error ending Command Buffer, skipping Logical Device Tests " << endResult.error().get_message();

                    if (auto submitResult = graphicsQueues[0].submit2(*commandBufferResult); !submitResult)
                        GTEST_SKIP() << "Error submiting Command Buffer, skipping Logical Device Tests " << submitResult.error().get_message();

                    if (!graphicsQueues[0].wait_idle())
                        GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";
                    if (!logicalDevice->wait_idle())
                        GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";

                    GTEST_ASSERT_TRUE(bufferResult->flush());
                    GTEST_ASSERT_TRUE(bufferResult->invalidate());
                    auto ptr = *reinterpret_cast<std::array<uint8_t, testBuffSize>*>(bufferResult->mapped_data());

                    if (ptr[0] == 255 && ptr[3] == 255)
                        counter++;
                }
                EXPECT_EQ(counter, testSize);
            }
            TEST_F(LogicalDeviceTestFixture, MinimalGraphicsTest)
            {
                auto graphicsQueues = logicalDevice->get_queues(Queue::Type::Graphics);
                {
                    GTEST_ASSERT_TRUE(graphicsQueues[0].wait_idle()) << "Error waiting on Graphics queue, skipping Logical Device Tests";
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
                    GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";

                auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

                if (!setPoolResult)
                    GTEST_SKIP() << "Error creating descriptor set pool, skipping Logical Device Tests";

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
                    GTEST_SKIP() << "Error creating push descriptor set layout, skipping Logical Device Tests";


                auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { static_cast<DescriptorSetLayout*>(&*setLayoutResult), 1 });

                if (!pipelineLayoutResult)
                    GTEST_SKIP() << "Error creating pipelineLayout, skipping Logical Device Tests";

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
                        .format{ rtFormat},
                        .dedicatedAllocation{ true },
                    });

                if (!rtResult)
                    GTEST_SKIP() << "Error creating Render Target, skipping Logical Device Tests " << rtResult.error().get_message();

                auto renderPassResult = SingleRenderPass::create(*logicalDevice, 
                    SingleRenderPass::Params{ .attachments { SingleRenderPass::Attachment{
                        .load{ LoadOp::Clear },
                        .store{ StoreOp::Store },
                        .format{static_cast<Format>(rtFormat)},
                        .samples{ rtSamples },
                        .initialLayout{ Layout::Undefined },
                        .finalLayout{ Layout::TransferSrc },
                        .clearValue{0x00FFFF00u}
                    }}});

                if (!renderPassResult)
                    GTEST_SKIP() << "Error creating RenderPass, skipping Logical Device Tests " << renderPassResult.error().get_message();

                auto framebufferResult = Framebuffer::create(*logicalDevice, { .renderPass {*renderPassResult}, .attachments{rtResult->get_image_view(0)}, .width {rtWidth},. height {rtHeight} });

                if (!framebufferResult)
                    GTEST_SKIP() << "Error creating Framebuffer, skipping Logical Device Tests " << framebufferResult.error().get_message();

                auto vertShaderCode = read_binary_file("shader/triangle_test_vert.spv");
                if (vertShaderCode.empty())
                    GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                auto vertShaderResult = Shader::create(*logicalDevice, vertShaderCode);
                if (!vertShaderResult)
                    GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";
                //ShaderCode can be freed now

                auto vertShaderInstance = vertShaderResult->create_shader_instance();


                auto fragShaderCode = read_binary_file("shader/triangle_test_frag.spv");
                if (fragShaderCode.empty())
                    GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                auto fragShaderResult = Shader::create(*logicalDevice, fragShaderCode);
                if (!fragShaderResult)
                    GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";
                //ShaderCode can be freed now

                auto fragShaderInstance = fragShaderResult->create_shader_instance();

                auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
                if (!pipelineCacheResult)
                    GTEST_SKIP() << "Error creating pipeline Cache, skipping Logical Device Tests";

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
                   GTEST_SKIP() << "Error creating pipeline, skipping Logical Device Tests";

                //ShaderModule, PipelineCache and PipelineLayout could be destroyed now

                auto commandPoolResult = CommandPool::create(*logicalDevice, graphicsQueues[0]);
                if (!commandPoolResult)
                    GTEST_SKIP() << "Error creating Command Pool, skipping Logical Device Tests " << commandPoolResult.error().get_message();

                auto commandBufferResult = commandPoolResult->allocate_command_buffer();
                constexpr auto testSize = 5;

                if (!commandBufferResult)
                    GTEST_SKIP() << "Error allocating Command Buffer, skipping Logical Device Tests " << commandBufferResult.error().get_message();

                constexpr size_t testBuffSize = format_byte_size(rtFormat) * rtWidth * rtHeight;

                auto bufferResult = StorageBuffer::create(*logicalDevice, Buffer::Options{ .size {testBuffSize}, .dedicatedAllocation = true, .mapable = true });

                if (!bufferResult)
                    GTEST_SKIP() << "Error creating buffer, skipping Logical Device Tests " << bufferResult.error().get_message();

                uint32_t counter = 0;
                for (auto i = 0; i < testSize; ++i) {
                    if(i != 0)
                        if (auto result = commandPoolResult->reset(); !result)
                            GTEST_SKIP() << "Error reseting Command Pool, skipping Logical Device Tests";

                    if (auto beginResult = commandBufferResult->begin(); !beginResult)
                        GTEST_SKIP() << "Error beginning Command Buffer, skipping Logical Device Tests " << beginResult.error().get_message();


                    auto bind = commandBufferResult->bind_pipeline(*pipelineResult);
                    bind.begin_render_pass(*renderPassResult, *framebufferResult);
                    bind.draw(3);
                    bind.end_render_pass();
                    commandBufferResult->barrier2();
                    if (auto endResult = commandBufferResult->end(); !endResult)
                        GTEST_SKIP() << "Error ending Command Buffer, skipping Logical Device Tests " << endResult.error().get_message();
                    if (auto submitResult = graphicsQueues[0].submit2(*commandBufferResult); !submitResult)
                        GTEST_SKIP() << "Error submiting Command Buffer, skipping Logical Device Tests " << submitResult.error().get_message();
                    if (!graphicsQueues[0].wait_idle())
                        GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";
                    if (auto result = commandPoolResult->reset(); !result)
                        GTEST_SKIP() << "Error reseting Command Pool, skipping Logical Device Tests";

                    if (auto beginResult = commandBufferResult->begin(); !beginResult)
                        GTEST_SKIP() << "Error beginning Command Buffer, skipping Logical Device Tests " << beginResult.error().get_message();

                    commandBufferResult->barrier2();
                    commandBufferResult->copy_image_to_buffer(*rtResult, Layout::TransferSrc, *bufferResult);
                    commandBufferResult->barrier2();

                    if (auto endResult = commandBufferResult->end(); !endResult)
                        GTEST_SKIP() << "Error ending Command Buffer, skipping Logical Device Tests " << endResult.error().get_message();

                    if (auto submitResult = graphicsQueues[0].submit2(*commandBufferResult); !submitResult)
                        GTEST_SKIP() << "Error submiting Command Buffer, skipping Logical Device Tests " << submitResult.error().get_message();

                    if (!graphicsQueues[0].wait_idle())
                        GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";
                    if (!logicalDevice->wait_idle())
                        GTEST_SKIP() << "Error waiting on Queue, skipping Logical Device Tests";

                    GTEST_ASSERT_TRUE(bufferResult->flush());
                    GTEST_ASSERT_TRUE(bufferResult->invalidate());
                    auto ptr = *reinterpret_cast<std::array<uint8_t, testBuffSize>*>(bufferResult->mapped_data());

                    if (ptr[0] == 255 && ptr[3] == 255)
                        counter++;
                }
                EXPECT_EQ(counter, testSize);
            }
        }
	}
}