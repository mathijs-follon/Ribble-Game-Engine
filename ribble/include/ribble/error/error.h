//
// Created by Mathijs Follon on 2/17/26.
//

#ifndef RIBBLE_ERROR_H
#define RIBBLE_ERROR_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>


namespace ribble::error {

	class Error
	{
	public:
		struct ErrorLocation {
#ifdef RIBBLE_DEBUG
			const char* file;
			size_t line;
#endif
		};

		explicit Error(uint8_t failure, bool fatal = false);
		Error(uint8_t failure, const ErrorLocation& errorLocation, bool fatal = false);
		virtual ~Error() = default;

		template<typename T>
		requires std::is_enum_v<T> || std::convertible_to<uint8_t, T>
		T failure() const {
			return static_cast<T>(m_failure);
		}

		[[nodiscard]] std::optional<ErrorLocation> location() const;
		[[nodiscard]] bool is_fatal() const { return m_isFatal; }

		using ErrorCallback = std::function<void(const Error&)>;
		
		static void SetCallback(ErrorCallback callback);
		static void Throw(uint8_t failure, bool fatal = false);
		static void Throw(uint8_t failure, const ErrorLocation& errorLocation, bool fatal = false);

	protected:
		static ErrorCallback s_callback;
		bool m_isFatal;
		uint8_t m_failure;
		ErrorLocation m_location;
	};

} // namespace ribble::error


#endif //RIBBLE_ERROR_H