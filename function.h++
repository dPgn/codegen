#ifndef CODEGEN_FUNCTION_H
#define CODEGEN_FUNCTION_H

#include "codeseg.h++"

namespace codegen
{
	template<class T> class function { };

	// TODO: Rule of 5
	template<class R, class... ARGS> class function<R(ARGS...)>
	{
		codeseg *code = nullptr;
		std::size_t offset = 0;

	public:

		function(const function &other)
		{
			*this = other;
		}

		function(function &&other)
		{
			*this = other;
		}

		function(codeseg *code = nullptr, std::size_t offset = 0)
		{
			this->code = code;
			this->offset = offset;
		}

		virtual ~function()
		{
			if (code) code->remove_ref();
		}

		R operator()(ARGS... args)
		{
			return reinterpret_cast<R(*)(ARGS...)>((byte *)*code + offset)(args...);
		}

		function &operator=(const function &other)
		{
			code = other.code;
			offset = other.offset;
			code->add_ref();
			return *this;
		}

		function &operator=(function &&other)
		{
			code = other.code;
			offset = other.offset;
			other.code = nullptr;
			return *this;
		}
	};
}

#endif
