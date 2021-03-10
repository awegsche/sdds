// sddslib.cpp : Defines the entry point for the application.
//

#include "sddslib.h"
#include <fstream>
#include <sstream>
#include "endian.h"

#include <spdlog/spdlog.h>

namespace sdds {
	SddsFile::SddsFile(const std::string& filename)
	{
		load(filename);
	}

	void SddsFile::load(const std::string& filename, size_t bufsize)
	{
		SPDLOG_DEBUG("start reading sdds file");
		std::ifstream file(filename, std::ios_base::binary);
		std::string versionstring;

		std::getline(file, versionstring);

		if (versionstring != "SDDS1") {
			throw std::runtime_error("only SDDS file verion 1 is supported at the moment");
			return;
		}

		char* buffer = new char[bufsize];

		while (true) {
			char chck = file.peek();
			if (chck == '!') {
				file.get(buffer, bufsize, '\n');
				file.get();
				continue;
			}
			if (chck != '&') break;

			auto p1 = file.tellg();
			file.get(buffer, bufsize, '\n');
			file.get();
			auto p2 = file.tellg();

			SPDLOG_DEBUG("parsing header line {}", buffer);
			namelists::namelist nl;
			nl.get_descriptor(buffer, p2 - p1);

			data_.push_back(nl);
		}
		SPDLOG_DEBUG("dropped out of header reading");

		// skip column count
		int ncol;
		file.read(reinterpret_cast<char*>(&ncol), 4);
		SPDLOG_DEBUG("ncols = {}", ncol);

		for (auto& nl : data_) {
			nl.read_value(file);
			std::stringstream ss;
			ss << nl;
			SPDLOG_DEBUG("loaded namelist: {}", ss.str());
		}

		delete[] buffer;
	}

	namelists::namelist& SddsFile::get_namelist(const std::string& name)
	{
		auto found = std::find(data_.begin(), data_.end(), name);
		if (found != data_.end())
			return *found;
		throw std::runtime_error("namelist not found");
	}

	namespace namelists {

		namelist::name_list_type namelist::type_from_str(const char* name)
		{
			if (!strcmp(name, "parameter")) return Parameter;
			if (!strcmp(name, "array")) return SddsArray;
			return NDef;
		}

		void sdds::namelists::namelist::get_descriptor(char* buffer, std::streamsize n)
		{
			boost::cmatch match;
			boost::regex_match(buffer, match, ReDescriptor);

			boost::cmatch desc_properties;
			auto start = match[2].begin();
			auto end = match[2].end();

			namelist_datatype tt = sdds::namelists::NDef;

			if (boost::regex_search(start, end, desc_properties, Re_Prop_Name)) {
				name = std::string(desc_properties[1].begin(), desc_properties[1].end());
			}
			if (boost::regex_search(start, end, desc_properties, Re_Prop_Type)) {
				tt = datatype_from_str(desc_properties[1].begin(), desc_properties[1].end());
			}
			if (match[1] == Str_Parameter) {
				type = Parameter;
				payload = parameter{tt};
			}
			else if (match[1] == Str_Array) {
				type = SddsArray;
				payload = sddsarray{tt};
			}
		}

		void namelist::read_value(std::istream& stream) {
			switch (type) {
			case Parameter:
				std::get<parameter>(payload).read(stream);
				break;
			case SddsArray:
				std::get<sddsarray>(payload).read(stream);
				break;
			}
		}

		bool namelist::operator==(const std::string& str)
		{
			return name == str;
		}

		namelist_datatype datatype_from_str(const char* start, const char* end)
		{
			if (boost::regex_match(start, end, Re_Type_Float)) return Float;
			if (boost::regex_match(start, end, Re_Type_Double)) return Double;
			if (boost::regex_match(start, end, Re_Type_Long)) return Long;
			if (boost::regex_match(start, end, Re_Type_String)) return String;
			return NDef;
		}

		void parameter::read(std::istream& stream)
		{
			switch (type) {
			case Double:
				double d;
				stream.read(reinterpret_cast<char*>(&d), sizeof(double));
				value = bswap64(d);
				break;
			case Float:
				float f;
				stream.read(reinterpret_cast<char*>(&f), sizeof(float));
				value = bswap32(f);
				break;
			case Long:
				long l;
				stream.read(reinterpret_cast<char*>(&l), sizeof(long));
				value = bswap32(l);
				break;
			case String:
				long length;
				stream.read(reinterpret_cast<char*>(&length), sizeof(long));
				length = bswap32(l);
				value = std::string{};
				auto& s = std::get<std::string>(value);
				s.resize(length);
				stream.read(&s[0], length);
				break;
			}
		}

		void sddsarray::read(std::istream& stream)
		{
			int length;
			stream.read(reinterpret_cast<char*>(&length), sizeof(long));
			length = bswap32(length);

			SPDLOG_DEBUG("length of array: {}", length);

			switch (type) {
			case Long:
			{
				payload = std::vector<long>();
				auto& v = std::get<std::vector<long>>(payload);
				v.reserve(length);
				for (int i = 0; i < length; i++)
				{
					long number;
					stream.read(reinterpret_cast<char*>(&number), sizeof(long));
					v.push_back(bswap32(number));
				}
				break;
			}
			case Float:
			{
				payload = std::vector<float>();
				auto& v = std::get<std::vector<float>>(payload);
				v.reserve(length);
				for (int i = 0; i < length; i++)
				{
					float number;
					stream.read(reinterpret_cast<char*>(&number), sizeof(float));
					v.push_back(bswap32(number));
				}
				break;
			}
			case Double:
			{
				payload = std::vector<double>();
				auto& v = std::get<std::vector<double>>(payload);
				v.reserve(length);
				for (int i = 0; i < length; i++)
				{
					double number;
					stream.read(reinterpret_cast<char*>(&number), sizeof(double));
					v.push_back(bswap64(number));
				}
				break;
			}
			case String:
			{
				payload = std::vector<std::string>();
				auto& v = std::get<std::vector<std::string>>(payload);
				v.reserve(length);
				for (int i = 0; i < length; i++)
				{
					long slen;
					stream.read(reinterpret_cast<char*>(&slen), sizeof(long));
					slen = bswap32(slen);
					std::string s{};
					s.resize(slen);
					stream.read(&s[0], slen);
					v.push_back(s);
				}
				break;
			}
			}
		}

	}
}

std::ostream& operator<<(std::ostream& os, const sdds::namelists::namelist& list)
{
	switch (list.type) {
	case sdds::namelists::namelist::Parameter:
		return os << "Paramter " << list.name << " " << std::get<sdds::namelists::parameter>(list.payload);
	case sdds::namelists::namelist::SddsArray:
		return os << "Array " << list.name << " " << std::get<sdds::namelists::sddsarray>(list.payload);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const sdds::namelists::parameter& param)
{
	switch (param.type) {
	case sdds::namelists::namelist_datatype::Float:
		return os << std::get<float>(param.value);
	case sdds::namelists::namelist_datatype::Double:
		return os << std::get<double>(param.value);
	case sdds::namelists::namelist_datatype::Long:
		return os << std::get<long>(param.value);
	case sdds::namelists::namelist_datatype::String:
		return os << "'" << std::get<std::string>(param.value) << "'";
	case sdds::namelists::namelist_datatype::NDef:
		return os << "<NDef data type>";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const sdds::namelists::sddsarray& param)
{
	os << "[ ";
	switch (param.type) {
	case sdds::namelists::namelist_datatype::Float:
	{
		auto& arr = std::get<std::vector<float>>(param.payload);
		for (int i = 0; i < 10 && i < arr.size(); i++)
			os << arr[i] << " ";
		break;

	}
	case sdds::namelists::namelist_datatype::Double:
	{
		auto& arr = std::get<std::vector<double>>(param.payload);
		for (int i = 0; i < 10 && i < arr.size(); i++)
			os << arr[i] << " ";
		break;

	}
	case sdds::namelists::namelist_datatype::Long:
	{
		auto& arr = std::get<std::vector<long>>(param.payload);
		for (int i = 0; i < 10 && i < arr.size(); i++)
			os << arr[i] << " ";
		break;
	}
	case sdds::namelists::namelist_datatype::String:
	{
		auto& arr = std::get<std::vector<std::string>>(param.payload);
		for (int i = 0; i < 10 && i < arr.size(); i++)
			os << "'" << arr[i] << "' ";
		break;
	}
	case sdds::namelists::namelist_datatype::NDef:
		return os << "<NDef data type>";
	}
	os << " ...]";
	return os;
}
