#ifndef UID_H
#define UID_H
#pragma once
#include <cstdint>
#include <atomic>
namespace Utility {
	class UID {
	public:
		UID() {
			m_id = s_counter++;
		}
		operator uint64_t() const {
			return m_id;
		}
	private:
		uint64_t m_id = ~0Ui64;
		static std::atomic<uint64_t> s_counter;
	};
}

#endif