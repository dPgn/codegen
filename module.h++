#ifndef CODEGEN_MODULE_H
#define CODEGEN_MODULE_H

#include "function.h++"

namespace codegen
{
	class module
	{
	protected:

		std::vector<byte> code;
	};

	template<class T> class function_module { };

	template<class R, class... A> class function_module<R(A...)> : public module
	{

	public:

		function_module() { }

		function_module(const std::vector<byte> &content)
		{
			code = content;
		}

		function<R(A...)> link_function()
		{
			return function<R(A...)>(new codeseg(code));
		}
	};
}

#endif
