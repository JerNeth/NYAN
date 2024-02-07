import NYANVulkanWrapper;
import NYANLog;

#include <memory>
#include <array>
#include <vector>
#include <fstream>
#include <filesystem>

#include <gtest/gtest.h>

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
                    Instance::ExtensionSettings optionalExtensions{};
                    auto applicationName = " LogicalDeviceTests";
                    auto engineName = " LogicalDeviceTests";
                    auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
                    if (!instanceCreateResult)
                        GTEST_SKIP() << "Could not create Instance, skipping  Logical Device Tests";
                    instance = std::make_unique<Instance>(std::move(*instanceCreateResult));

                    PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions{

                    };

                    auto physicalDeviceResult = instance->select_physical_device(std::nullopt, requiredDeviceExtensions);

                    if (!physicalDeviceResult)
                        GTEST_SKIP() << "Could not create Instance, skipping  Logical Device Tests";

                    auto physicalDevice = *physicalDeviceResult;

                    QueueContainer<float> queuePriorities;
                    queuePriorities[Queue::Type::Graphics].push_back(1.f);
                    queuePriorities[Queue::Type::Compute].push_back(1.f);
                    queuePriorities[Queue::Type::Transfer].push_back(1.f);

                    PhysicalDevice::Extensions extensions{
                        .pushDescriptors {1}
                    };
                    auto logicalDeviceCreateResult = LogicalDevice::create(*instance, physicalDevice,
                        nullptr, extensions, queuePriorities);
                    if (!logicalDeviceCreateResult)
                        GTEST_SKIP() << "Could not create LogicalDevice, skipping  Logical Device Tests";
                    logicalDevice = std::make_unique<LogicalDevice>(std::move(*logicalDeviceCreateResult));


                    if (logicalDevice->get_queues(Queue::Type::Graphics).size() != queuePriorities[Queue::Type::Graphics].size())
                        GTEST_SKIP() << "Error creating Graphics queue, skipping  Logical Device Tests";

                    if (logicalDevice->get_queues(Queue::Type::Compute).size() != queuePriorities[Queue::Type::Compute].size())
                        GTEST_SKIP() << "Error creating Compute queue, skipping  Logical Device Tests";

                    if (logicalDevice->get_queues(Queue::Type::Transfer).size() != queuePriorities[Queue::Type::Transfer].size())
                        GTEST_SKIP() << "Error creating Transfer queue, skipping  Logical Device Tests";
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Graphics);
                        //queues[0].wait_idle();
                    }
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Compute);
                        //queues[0].wait_idle();
                    }
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Transfer);
                        //queues[0].wait_idle();
                    }

                    auto setLayoutResult = DescriptorSetLayout::create(*logicalDevice, { 16, 16, 16, 16, 16, 16, 16 });

                    if (!setLayoutResult)
                        GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";

                    auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 21);

                    if (!setPoolResult)
                        GTEST_SKIP() << "Error creating descriptor set pool, skipping Logical Device Tests";

                    auto& sets = setPoolResult->get_sets();


                    auto pushSetLayoutResult = PushDescriptorSetLayout::create(*logicalDevice, DescriptorSetLayout::DescriptorInfo{
                            .storageBufferCount{ 1 },
                            .uniformBufferCount{ 0 },
                            .samplerCount{ 0 },
                            .sampledImageCount{ 0 },
                            .storageImageCount{ 0 },
                            .inputAttachmentCount{ 0 },
                            .accelerationStructureCount{ 0 }
                        });

                    if (!pushSetLayoutResult)
                        GTEST_SKIP() << "Error creating push descriptor set layout, skipping Logical Device Tests";

                    auto pipelineLayoutResult = PipelineLayout::create(logicalDevice->get_device(), logicalDevice->get_deletion_queue(), { static_cast<DescriptorSetLayout*>(& *pushSetLayoutResult), 1});

                    if (!pipelineLayoutResult)
                        GTEST_SKIP() << "Error creating pipelineLayout, skipping Logical Device Tests";


                    auto shaderCode = read_binary_file("shader/test_comp.spv");
                    if (shaderCode.empty())
                        GTEST_SKIP() << "Error reading shader, skipping Logical Device Tests";

                    auto shaderResult = Shader::create(*logicalDevice, shaderCode);
                    if (!shaderResult)
                        GTEST_SKIP() << "Error creating shader, skipping Logical Device Tests";

                    auto shaderInstance = shaderResult->create_shader_instance();
                    shaderInstance.set_spec_constant(0, 1u);
                    shaderInstance.set_spec_constant(1, 1u);
                    shaderInstance.set_spec_constant(2, 1u);
                    shaderInstance.set_spec_constant(3, 5u);


                    auto pipelineCacheResult = PipelineCache::create(*logicalDevice, {});
                    if (!pipelineCacheResult)
                        GTEST_SKIP() << "Error creating pipeline Cache, skipping Logical Device Tests";

                    auto pipelineResult = ComputePipeline::create(logicalDevice->get_device(), *pipelineLayoutResult , shaderInstance, *pipelineCacheResult);

                    if (!pipelineResult)
                        GTEST_SKIP() << "Error creating pipeline, skipping Logical Device Tests";

                    auto bufferResult = StorageBuffer::create(*logicalDevice, 65536);

                    //auto shaderInstanceId = shaderStorage.add_instance(shaderId);
                    //m_shaderInstanceMapping[entry.path().stem().string()] = shaderInstanceId;

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
            TEST_F(LogicalDeviceTestFixture, a)
            {

            }
        }
	}
}