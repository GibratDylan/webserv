#ifndef ACONFIG_HPP
#define ACONFIG_HPP

#include <map>
#include <string>

class AConfig {
   public:
	AConfig();
	AConfig(const AConfig& src);
	virtual ~AConfig();

	AConfig& operator=(const AConfig& rhs);

	virtual const std::string& getDirective(const std::string& interface,
											const std::string& location,
											const std::string& name);

	const std::map<std::string, std::string>& getConfig() const;
	void setConfig(const std::map<std::string, std::string>& newConfig);

   private:
	std::map<std::string, std::string> _config;
};

#endif
