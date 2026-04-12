/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:11 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/ConfigParser.hpp"

#include <stdexcept>

#include "../../include/config/DirectiveRegistry.hpp"
#include "../../include/config/GlobalConfig.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"

namespace {

class Tokenizer {
   public:
	struct Token {
		enum Type {
			WORD,
			SEMICOLON,
			LBRACE,
			RBRACE,
			END,
		};

		Type type;
		std::string value;
	};

	Tokenizer(const std::string& content) : _content(content), _pos(0) {}

	Token next() {
		skipWhitespaceAndComments();
		if (_pos >= _content.size()) {
			Token t;
			t.type = Token::END;
			return t;
		}

		const char c = _content[_pos];
		if (c == ';') {
			_pos++;
			Token t;
			t.type = Token::SEMICOLON;
			return t;
		}
		if (c == '{') {
			_pos++;
			Token t;
			t.type = Token::LBRACE;
			return t;
		}
		if (c == '}') {
			_pos++;
			Token t;
			t.type = Token::RBRACE;
			return t;
		}

		Token t;
		t.type = Token::WORD;
		t.value = readWord();
		return t;
	}

	Token peek() {
		const size_t saved = _pos;
		Token t = next();
		_pos = saved;
		return t;
	}

   private:
	void skipWhitespaceAndComments() {
		while (_pos < _content.size()) {
			const char c = _content[_pos];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
				_pos++;
				continue;
			}
			if (c == '#') {
				skipComment();
				continue;
			}
			break;
		}
	}

	void skipComment() {
		while (_pos < _content.size() && _content[_pos] != '\n') {
			_pos++;
		}
	}

	std::string readWord() {
		if (_content[_pos] == '"') {
			return readQuotedWord();
		}
		const size_t start = _pos;
		while (_pos < _content.size()) {
			const char c = _content[_pos];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ';' ||
				c == '{' || c == '}' || c == '#') {
				break;
			}
			_pos++;
		}
		return _content.substr(start, _pos - start);
	}

	std::string readQuotedWord() {
		_pos++;	 // skip opening quote
		const size_t start = _pos;
		while (_pos < _content.size() && _content[_pos] != '"') {
			_pos++;
		}
		if (_pos >= _content.size()) {
			throw std::runtime_error("Error: Double quotes not closed");
		}
		const std::string v = _content.substr(start, _pos - start);
		_pos++;	 // skip closing quote
		return v;
	}

	const std::string& _content;
	size_t _pos;
};

static std::vector<std::string> readArgs(Tokenizer& tokenizer) {
	std::vector<std::string> args;
	Tokenizer::Token t = tokenizer.peek();
	while (t.type == Tokenizer::Token::WORD) {
		args.push_back(tokenizer.next().value);
		t = tokenizer.peek();
	}
	return args;
}

static void parseLocationBlock(Tokenizer& tokenizer,
							   DirectiveRegistry& registry,
							   ServerConfig& serverCfg,
							   const std::string& path) {
	Config locCfg(serverCfg);
	locCfg.location_path = path;
	locCfg.root_explicitly_set = false;

	while (true) {
		Tokenizer::Token t = tokenizer.peek();
		if (t.type == Tokenizer::Token::RBRACE) {
			tokenizer.next();
			break;
		}
		if (t.type == Tokenizer::Token::END) {
			throw std::runtime_error(
				"Error: Unmatched braces in location block");
		}
		if (t.type != Tokenizer::Token::WORD) {
			throw std::runtime_error("Error: Invalid token in location block");
		}

		const std::string name = tokenizer.next().value;
		const std::vector<std::string> args = readArgs(tokenizer);
		Tokenizer::Token endTok = tokenizer.next();
		if (endTok.type == Tokenizer::Token::LBRACE ||
			endTok.type == Tokenizer::Token::RBRACE) {
			throw std::runtime_error("Error: Bracket in location");
		}
		if (endTok.type != Tokenizer::Token::SEMICOLON) {
			throw std::runtime_error(
				"Error: Directive without semicolon terminator");
		}
		registry.apply(DirectiveRegistry::SCOPE_LOCATION, name, locCfg, args);
	}

	if (serverCfg.location.find(path) != serverCfg.location.end()) {
		throw std::runtime_error("Error: Duplicate location");
	}
	serverCfg.location.insert(std::make_pair(path, locCfg));
}

static void parseServerBlock(Tokenizer& tokenizer, DirectiveRegistry& registry,
							 ServerConfig& serverCfg) {
	bool locationSeen = false;

	while (true) {
		Tokenizer::Token t = tokenizer.peek();
		if (t.type == Tokenizer::Token::RBRACE) {
			tokenizer.next();
			break;
		}
		if (t.type == Tokenizer::Token::END) {
			throw std::runtime_error("Error: Unmatched braces in server block");
		}
		if (t.type != Tokenizer::Token::WORD) {
			throw std::runtime_error("Error: Invalid token in server block");
		}

		const std::string name = tokenizer.next().value;
		const std::vector<std::string> args = readArgs(tokenizer);
		Tokenizer::Token endTok = tokenizer.next();

		if (name == "location") {
			if (args.size() != 1) {
				throw std::runtime_error(
					"Error: Location requires a path and an optional modifier");
			}
			if (endTok.type != Tokenizer::Token::LBRACE) {
				throw std::runtime_error("Error: Location require brackets");
			}
			locationSeen = true;
			parseLocationBlock(tokenizer, registry, serverCfg, args[0]);
			continue;
		}

		if (locationSeen) {
			throw std::runtime_error("Error: Unknown directive in server '" +
									 name + "'");
		}

		if (endTok.type != Tokenizer::Token::SEMICOLON) {
			throw std::runtime_error(
				"Error: Directive without semicolon terminator");
		}
		registry.apply(DirectiveRegistry::SCOPE_SERVER, name, serverCfg, args);
	}

	Logger::debug(std::string(" Server directive parsed on port ") +
				  toString(serverCfg.port));
}

}  // namespace

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

GlobalConfig* ConfigParser::parse(const std::string& filename) const {
	DirectiveRegistry registry;
	const std::string content = FileSystem::readFile(filename);
	Tokenizer tokenizer(content);

	GlobalConfig* global = new GlobalConfig();
	bool serverSeen = false;

	while (true) {
		Tokenizer::Token t = tokenizer.peek();
		if (t.type == Tokenizer::Token::END) {
			break;
		}
		if (t.type == Tokenizer::Token::SEMICOLON) {
			throw std::runtime_error("Error: Empty directive before semicolon");
		}
		if (t.type == Tokenizer::Token::LBRACE ||
			t.type == Tokenizer::Token::RBRACE) {
			throw std::runtime_error(
				"Error: Bracket without directive in global");
		}

		const std::string name = tokenizer.next().value;
		const std::vector<std::string> args = readArgs(tokenizer);
		Tokenizer::Token endTok = tokenizer.next();

		if (name == "server") {
			serverSeen = true;
			if (endTok.type != Tokenizer::Token::LBRACE) {
				throw std::runtime_error("Error: Server require brackets");
			}

			ServerConfig serverCfg(*global);
			parseServerBlock(tokenizer, registry, serverCfg);

			for (std::map<int, ServerConfig>::iterator it =
					 global->server.begin();
				 it != global->server.end(); ++it) {
				if (it->second.port == serverCfg.port &&
					it->second.host == serverCfg.host) {
					delete global;
					throw std::runtime_error(
						"Error: Duplicate server address detected");
				}
			}

			global->server.insert(std::make_pair(serverCfg.port, serverCfg));
			continue;
		}

		if (serverSeen) {
			delete global;
			throw std::runtime_error("Error: Unknown directive in global '" +
									 name + "'");
		}

		if (endTok.type != Tokenizer::Token::SEMICOLON) {
			delete global;
			throw std::runtime_error(
				"Error: Bracket without directive in global");
		}

		registry.apply(DirectiveRegistry::SCOPE_GLOBAL, name, *global, args);
	}

	if (global->server.empty()) {
		delete global;
		throw std::runtime_error("Error: Config file need at least one server");
	}

	if (Logger::isDebugEnabled()) {
		Logger::debug(" Parsing config file:\n" + global->printDirectives());
	}

	return global;
}
