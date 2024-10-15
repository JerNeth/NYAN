module;

export module NYANRenderer:ModuleSettings;
import NYANAssert;

#ifdef NDEBUG
export constexpr inline auto assert = nyan::assert::AssertionsDisabled<nyan::assert::assertionExitMode, nyan::assert::assertionLoggingBehavior, nyan::assert::assertionDebugBehavior>;
#else
export constexpr inline auto assert = nyan::assert::AssertionsEnabled<nyan::assert::assertionExitMode, nyan::assert::assertionLoggingBehavior, nyan::assert::assertionDebugBehavior>;
#endif