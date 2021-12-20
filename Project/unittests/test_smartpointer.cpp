#define CATCH_CONFIG_MAIN
#include "smartpointer.h"
#include "catch.hpp"

TEST_CASE("Shared pointer")
{
	SECTION("shared pointer initialization with make shared")
	{
		util::shared_ptr<int> ptr1;
		CHECK(ptr1.get() == 0);
		ptr1 = util::make_shared<int>(5);
		CHECK(*ptr1 == 5);
		CHECK(*(reinterpret_cast<size_t*>(ptr1.get()) - 2) == 1); //CHECK REF COUNT
	}
	SECTION("shared pointer initialization with new")
	{
		util::shared_ptr<int> ptr1 = new int(12);
		CHECK(*ptr1 == 12);
		CHECK(**(reinterpret_cast<size_t const**>(&ptr1) + 1) == 1);
	}
	SECTION("shared pointer copy")
	{
		util::shared_ptr<int> ptr1 = util::make_shared<int>(5);
		util::shared_ptr<int> ptr2 = ptr1;
		CHECK(*ptr2 == 5);
		CHECK(*(reinterpret_cast<size_t*>(ptr1.get()) - 2) == 2);
		ptr1.reset();
		CHECK(*(reinterpret_cast<size_t*>(ptr2.get()) - 2) == 1);

		util::shared_ptr<int> ptr3 = new int(12);
		CHECK(*ptr3 == 12);
		CHECK(**(reinterpret_cast<size_t const**>(&ptr3) + 1) == 1);
		util::shared_ptr<int> ptr4 = ptr3;
		CHECK(**(reinterpret_cast<size_t const**>(&ptr3) + 1) == 2);
	}
}
TEST_CASE("Weak pointer")
{
	SECTION("weak pointer initialization with make shared")
	{
		util::weak_ptr<int> wptr1;
		CHECK(wptr1.get() == 0);
		util::shared_ptr<int> sptr = util::make_shared<int>(5);
		wptr1 = sptr;
		CHECK(*(reinterpret_cast<size_t*>(sptr.get()) - 2) == 1);
		CHECK(*(reinterpret_cast<size_t*>(wptr1.get()) - 1) == 1);
		CHECK(*wptr1.lock() == 5);
		wptr1.reset();
		CHECK(*(reinterpret_cast<size_t*>(sptr.get()) - 2) == 1);
		CHECK(*(reinterpret_cast<size_t*>(sptr.get()) - 1) == 0);
	}
	SECTION("weak pointer initialization with new")
	{
		util::weak_ptr<int> wptr1;
		CHECK(wptr1.get() == 0);
		util::shared_ptr<int> sptr = new int(5);
		wptr1 = sptr;
		CHECK(**(reinterpret_cast<size_t const**>(&sptr) + 1) == 1);
		CHECK(*(*(reinterpret_cast<size_t const**>(&sptr) + 1) + 1) == 1);
		CHECK(*wptr1.lock() == 5);
		wptr1.reset();
		CHECK(**(reinterpret_cast<size_t const**>(&sptr) + 1) == 1);
		CHECK(*(*(reinterpret_cast<size_t const**>(&sptr) + 1) + 1) == 0);
	}
	SECTION("weak pointer copy copy")
	{
		util::shared_ptr<int> sptr1 = util::make_shared<int>(5);
		util::weak_ptr<int> wptr1 = sptr1;
		CHECK(*(reinterpret_cast<size_t*>(sptr1.get()) - 2) == 1);
		CHECK(*(reinterpret_cast<size_t*>(wptr1.get()) - 1) == 1);
		util::weak_ptr<int> wptr2 = wptr1;
		CHECK(*(reinterpret_cast<size_t*>(sptr1.get()) - 2) == 1);
		CHECK(*(reinterpret_cast<size_t*>(wptr1.get()) - 1) == 2);
	}
}
TEST_CASE("Unique pointer")
{
	SECTION("unique pointer initialization with make unique")
	{
		util::unique_ptr<int> ptr1;
		CHECK(ptr1.get() == 0);
		ptr1 = util::make_unique<int>(5);
		CHECK(*ptr1 == 5);
	}
	SECTION("unique pointer initialization with new")
	{
		util::unique_ptr<int> ptr1;
		CHECK(ptr1.get() == 0);
		ptr1 = new int(5);
		CHECK(*ptr1 == 5);
	}
	SECTION("unique pointer move semantic")
	{
		util::unique_ptr<int> ptr1 = util::make_unique<int>(5);
		CHECK(*ptr1 == 5);
		util::unique_ptr<int> ptr2 = util::move(ptr1);
		CHECK(ptr1.get() == 0);
		CHECK(*ptr2 == 5);
	}
}