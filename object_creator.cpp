// object_creator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "boost_spirit_json/json_loader.h"
#include <boost/any.hpp>


class Constructor {
public:
	template <typename T>
	static void set(const std::function<T(const Component&)>& r)
	{
		instance()._set(r);
	}

	template <typename T>
	static T get(const Component& x)
	{
		return boost::any_cast<T>(instance()._rules[typeid(T).name()](x));
	}

private:
	static Constructor& instance() {
		static Constructor _this;
		return _this;
	}
private:
	Constructor() : _rules() {
		_set(std::function<double(const Component&)>([](const Component& x) {return boost::get<double>(x); }));
		_set(std::function<std::string(const Component&)>([](const Component& x) {return boost::get<std::string>(x); }));
	}

	template <typename T>
	void _set(const std::function<T(const Component&)>& r)
	{
		_rules[typeid(T).name()] = [=](const Component& x) {return boost::any(r(x)); };
	}
private:
	std::map<std::string, std::function<boost::any(const Component&)> > _rules;
};


template <typename T>
T as(const Component& x)
{
	return Constructor::get<T>(x);
}

template <typename T, typename A1>
void addConstructRule(const std::string& n1)
{
	Constructor::set(std::function<T(const Component&)>([=](const Component& c) {
		const Object& p = boost::get<Object>(c);
		const auto& d = p.components();
		return T(as<A1>(d.at(n1)));
	}));
}

template <typename T, typename A1, typename A2>
void addConstructRule(const std::string& n1, const std::string& n2)
{
	Constructor::set(std::function<T(const Component&)>([=](const Component& c) {
		const Object& p = boost::get<Object>(c);
		const auto& d = p.components();
		return T(as<A1>(d.at(n1)), as<A2>(d.at(n2)));
	}));
}


struct Person {
	Person(const std::string& name, const double age)
		: _name(name), _age(age)
	{
	}
	std::string _name;
	double _age;
};

struct Data {
	Data(const Person& person)
		: _person(person)
	{
	}
	Person _person;
};

int main()
{
	std::string jsonData = "{\"person\" : {\"name\" : \"testname\", \"age\" : 29}}";
	Object obj = load(jsonData);

	addConstructRule<Data, Person>("person");
	addConstructRule<Person, std::string, double>("name", "age");

	Data d = as<Data>(obj);

	std::cout << d._person._name << std::endl;
	std::cout << d._person._age << std::endl;
	return 0;
}

