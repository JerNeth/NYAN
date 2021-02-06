#ifndef UID_H
#define UID_H
#pragma once
#include <cstdint>
#include <atomic>
namespace Utility {
	using UID = uint64_t;
	class UIDC {
	public:
		UIDC() {
			static std::atomic<UID> s_counter;
			m_id = s_counter++;
		}
		operator UID() const noexcept {
			return m_id;
		}
		UID get_id() const noexcept{
			return m_id;
		}
	private:
		UID m_id = ~0Ui64;
	};
}

#endif