#include <vector>
#include <string>
#include <iostream>

template<typename T> struct MyAlloc;

template<>
struct MyAlloc<void>
{
public:
	typedef void *pointer;
	typedef const void *const_pointer;

	typedef void value_type;

	template<typename U> struct rebind { typedef MyAlloc<U> other; };
};

template<typename T>
struct MyAlloc
{
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;
	typedef T value_type;

	template<typename U> struct rebind { typedef MyAlloc<U> other; };

	MyAlloc() throw() {}
	MyAlloc(const MyAlloc &r) throw() {}
	template<typename U> MyAlloc(const MyAlloc<U> &r) throw() {}
	~MyAlloc() throw() {}

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	pointer allocate(size_type s, typename MyAlloc<void>::const_pointer hint = 0)
	{
		pointer result = (pointer)malloc(s * sizeof(value_type));
		std::cout << "allocated " << (void *)result << "\n";
		return result;
	}

	void deallocate(pointer p, size_type s)
	{
		std::cout << "deallocated " << (void *)p << "\n";
		free(p);
	}

	size_type max_size() const throw() { return 0x7FFFFFFF; }

	void construct(pointer p, const_reference val)
	{
		new((void*)p) value_type(val);
	}

	void destroy(pointer p)
	{
		((value_type*)p)->~value_type();
	}
};

void basic_string_destructor_isnt_called()
{
	typedef std::string MyStr_t;
	typedef std::vector<MyStr_t> MyVec_t;

	MyVec_t t;
	t.push_back("some-textsome-textsome-textsome-textsome-text");
	t.clear();
}

void basic_string_destructor_is_called()
{
	typedef std::basic_string<char,std::char_traits<char>, MyAlloc<char> > MyStr_t;
	typedef std::vector<MyStr_t> MyVec_t;

	MyVec_t t;
	t.push_back("some-textsome-textsome-textsome-textsome-text");
	t.clear();
}

int main(int argc, char **argv)
{
	basic_string_destructor_is_called();
	basic_string_destructor_isnt_called();

	return 0;
}