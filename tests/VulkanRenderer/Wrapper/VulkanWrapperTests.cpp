import NYANVulkanWrapper;
import NYANLog;

#include <memory>
#include <array>
#include <vector>

#include <gtest/gtest.h>

namespace nyan
{
	namespace vulkan
	{
        namespace wrapper {
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
                    queuePriorities[static_cast<size_t>(Queue::Type::Graphics)].push_back(1.f);
                    queuePriorities[static_cast<size_t>(Queue::Type::Compute)].push_back(1.f);
                    queuePriorities[static_cast<size_t>(Queue::Type::Transfer)].push_back(1.f);

                    PhysicalDevice::Extensions extensions{
                    };
                    auto logicalDeviceCreateResult = LogicalDevice::create(*instance, physicalDevice,
                        nullptr, extensions, queuePriorities);
                    if (!logicalDeviceCreateResult)
                        GTEST_SKIP() << "Could not create LogicalDevice, skipping  Logical Device Tests";
                    logicalDevice = std::make_unique<LogicalDevice>(std::move(*logicalDeviceCreateResult));


                    if (logicalDevice->get_queues(Queue::Type::Graphics).size() != queuePriorities[static_cast<size_t>(Queue::Type::Graphics)].size())
                        GTEST_SKIP() << "Error creating Graphics queue, skipping  Logical Device Tests";

                    if (logicalDevice->get_queues(Queue::Type::Compute).size() != queuePriorities[static_cast<size_t>(Queue::Type::Compute)].size())
                        GTEST_SKIP() << "Error creating Compute queue, skipping  Logical Device Tests";

                    if (logicalDevice->get_queues(Queue::Type::Transfer).size() != queuePriorities[static_cast<size_t>(Queue::Type::Transfer)].size())
                        GTEST_SKIP() << "Error creating Transfer queue, skipping  Logical Device Tests";
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Graphics);
                        queues[0].wait_idle();
                    }
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Compute);
                        queues[0].wait_idle();
                    }
                    {
                        auto& queues = logicalDevice->get_queues(Queue::Type::Transfer);
                        queues[0].wait_idle();
                    }

                    auto setLayoutResult = DescriptorSetLayout::create(*logicalDevice, { 16, 16, 16, 16, 16, 16 });

                    if (!setLayoutResult)
                        GTEST_SKIP() << "Error creating descriptor set layout, skipping Logical Device Tests";

                    auto setPoolResult = DescriptorPool::create(*logicalDevice, *setLayoutResult, 2);

                    if (!setPoolResult)
                        GTEST_SKIP() << "Error creating descriptor set pool, skipping Logical Device Tests";

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