#include <fstream>
#include <ParadigmDebug.h>

#define UNIT_TEST_PATH (std::string)"../ParadigmUnitTest/"

using namespace std;

namespace Core
{
	namespace Debug
	{
		SUnitTests::SUnitTests()
		{
			FindHeaders();
			ParseHeaders();
		}

#pragma	region CPPCode

		void SUnitTests::WriteAndCompile(const unsigned int loopTimes) const
		{
			std::ofstream file(UNIT_TEST_PATH + "ParadigmUnitTest.cpp", ios::out | ios::trunc);
			if (file)
			{
				file << "//File auto generated by SUnitTest Made by Paradigm development team\n";
				file << "#include <map>\n#include <string>\n#include <iostream>\n#include <exception>\n";
				file << "#include <Includes/Debug.h>" << std::endl;
				file << "#include <iomanip>" << std::endl;
				file << includes << std::endl;
				file << "int main()\n{\n";
				file << "unsigned int loopTimes = " + std::to_string(loopTimes) + ";\n";
				if (loopTimes > 1u)
					file << "\tPRINT(\"All tests will be executed \" + std::to_string(loopTimes) + \" times.\\n\", LogLvl::CYAN);\n";
				file << script;

				file << "\n\tsystem(\"pause\");\n\treturn 0;\n}";
				file.close();
			}

			///Compile here
			system("start ParadigmUnitTest.exe");
		}

		void SUnitTests::AddPieceOfCode(EUnitTestMode mode, std::string Macro, std::string& className)
		{
			std::string  testCode = "";
			std::string temp, funcName, parameters = "";
			unsigned int cursor = 1;

			//parse the func Name
			for (; cursor < Macro.length() && Macro[cursor] != ',' && cursor < Macro.size() && Macro[cursor] != ')'; ++cursor)
			{
				temp = Macro[cursor];
				if (temp != "(" && temp != ")" && temp != "," && temp != " ")
					funcName += temp;
			}

			if ((funcName != "" && Macro != ""))
			{
				if (className != "")
					funcName = className + "." + funcName;

				script += "//" + Macro + "\n";
				if (functionList.find(funcName) == functionList.end())
				{
					script += "\tsuccessList.insert({\"" + funcName + "\",\"\"});\n";
					functionList.insert({ funcName,true });
				}
				script += "\tif(successList[\"" + funcName + "\"] == \"\") {\n";


				if (mode == NOCRASH || mode == CRASH)
				{
					testCode += AddCrashRelativeCode(mode, funcName, Macro, cursor);
				}
				else if (mode == COMPARE || mode == COMPARE_scalarS)
				{
					testCode += AddCompareCode(mode, funcName, Macro, cursor);
				}
				else if (mode == ISTRUE || mode == ISFALSE)
				{
					testCode += AddBooleanRelativeCode(mode, funcName, Macro, cursor);
				}
				if (mode == CLASS)
				{
					std::string tempname;
					for (size_t i = 0; i < funcName.length(); ++i)
					{
						if (funcName.at(i) == ':')
							tempname.clear();
						else
							tempname += funcName.at(i);
					}

					className = tempname + "_UTest";
					//					script += "\n//" + Macro + "\n";
					script += "\ttry {\n";
					script += "\t\t" + funcName + " " + className;
					std::string temp, testCode = "\n";
					if (Macro[cursor] == ',')
						cursor++;
					temp = Macro.substr(cursor, Macro.length() - 1);
					temp = "(" + temp;
					cursor = 0;
					ParseParams(temp, cursor, parameters);
					if (cursor< 2)
					{
						parameters.pop_back();
						parameters.pop_back();
					}
					script += parameters +";\n";
				}
				else
					script += testCode + "\t}//end of if\n//end " + Macro + "\n\n";
			}
			else if (mode == END_CLASS)
			{
				script += "\n\t}\n\tcatch (const std::exception&  ecept) {\n\n";
				script += "\t\tPRINT(\"Unit test (CLASS) constructor for " + className + " result by a crash\\n\", LogLvl::Error);\n";
				script += "\t\tPRINT(std::string(ecept.what())+\"\\n\", LogLvl::Error);\n";
				script += "\t}//end of trycatch\n\t}//end of if\n";
				script += "//end (CLASS)\n\n";

				className = "";
			}

		}

		std::string SUnitTests::AddCrashRelativeCode(EUnitTestMode _mode, std::string _funcName, std::string _macro, unsigned int _cursor)
		{
			std::string temp, parameters = "", testCode = "\n";
			testCode += "\t\t" + _funcName;
			if (_macro[_cursor] == ',')
				_cursor++;
			temp = _macro.substr(_cursor, _macro.length() - 1);
			temp = "(" + temp;
			_cursor = 0;
			ParseParams(temp, _cursor, parameters);
			testCode += parameters + ";\n";
			if (_mode == NOCRASH)
			{
				temp = "\t\tsuccessList[\"" + _funcName + "\"] = \"Unit test (NOCRASH) for " + _funcName + " result by a crash with these input values : " + parameters + "\\n\"";
			}
			else if (_mode == CRASH)
			{
				testCode += "\t\tsuccessList[\"" + _funcName + "\"] = \"Unit test (CRASH) for " + _funcName + " dosen't crash with these input values : " + parameters + "\\n\";\n";
				temp = "";
			}

			return TryCatch(testCode, temp);
		}

		std::string SUnitTests::AddCompareCode(EUnitTestMode _mode, std::string _funcName, std::string _macro, unsigned int _cursor)
		{
			std::string temp, parameters = "", testCode = "", expected;
			if (_macro[_cursor] == ',')
				_cursor++;
			temp = _macro.substr(_cursor, _macro.length() - 1);
			temp = "(" + temp;
			_cursor = 0;
			ParseParams(temp, _cursor, parameters);
			testCode += "\t\tauto result = " + _funcName + parameters + ";\n";
			if (_mode == COMPARE)
				testCode += "\t\tif (result != ";
			else
				testCode += "\t\tif (!Equalf(result, ";
			_cursor++;
			ParseParams(temp, _cursor, expected);
			testCode += expected;
	//		std:cout << temp << std::endl;
			if (_mode == COMPARE_scalarS)
				testCode += ")";
			testCode += ") {\n";
			testCode += "\t\t\tstd::stringstream ss;\tss << std::setprecision(5) << result;\n";
			testCode += "\t\t\tsuccessList[\"" + _funcName + "\"] = \"Unit test (COMPARE) for " + _funcName + " didn't returns the expected value " + expected + ".\\n\t Actual result : \" + ss.str()+ \"\\n\";\n";
			testCode += "\t\t}\n";
			temp.clear();

			temp = "\t\tsuccessList[\"" + _funcName + "\"] = \"Unit test (COMPARE) for " + _funcName + " result by a crash with these input values : " + parameters + "\\n\"";

			return TryCatch(testCode, temp);
		}

		std::string SUnitTests::AddBooleanRelativeCode(EUnitTestMode _mode, std::string _funcname, std::string _macro, unsigned int _cursor)
		{
			std::string temp, parameters = "", testCode = "\n";
			testCode += "\t\tif (!" + _funcname;
			if (_macro[_cursor] == ',')
			{
				_cursor++;
				temp = _macro.substr(_cursor, _macro.length() - 1);
				temp = "(" + temp;
				_cursor = 0;
				ParseParams(temp, _cursor, parameters);
			}
			else
				testCode += "(";
			testCode += parameters + ") {\n";
			if (_mode == ISTRUE)
			{
				testCode += "\t\t\tsuccessList[\"" + _funcname + "\"] = \"Unit test (ISTRUE) for " + _funcname + " returns a non-true value with these input values : " + parameters + "\\n\";\n";
			}
			else if (_mode == ISFALSE)
			{
				testCode += "\t\t}\n";
				testCode += "\t\telse {\n";
				testCode += "\t\t\tsuccessList[\"" + _funcname + "\"] = \"Unit test (ISFALSE) for " + _funcname + " returns a true value with these input values : " + parameters + "\\n\";\n";
			}
			testCode += "\t\t}\n";

			temp = "\t\tsuccessList[\"" + _funcname + "\"] = \"Unit test for " + _funcname + " result by a crash with these input values : " + parameters + "\\n\"";
			return TryCatch(testCode, temp);
		}

		std::string SUnitTests::AddClassRelativeCode(EUnitTestMode _mode, std::string& _classname, std::string _macro, unsigned int& _cursor)
		{
			if (_mode == CLASS)
			{
				std::string tempname, parameters, funcName;
				for (size_t i = 0; i < funcName.length(); ++i)
				{
					if (funcName.at(i) == ':')
						tempname.clear();
					else
						tempname += funcName.at(i);
				}

				_classname = tempname + "_UTest";
				//					script += "\n//" + Macro + "\n";
				script += "\ttry {\n";
				script += "\t\t" + funcName + " " + _classname;
				std::string temp, testCode = "\n";
				if (_macro[_cursor] == ',')
					_cursor++;
				temp = _macro.substr(_cursor, _macro.length() - 1);
				temp = "(" + temp;
				_cursor = 0;
				ParseParams(temp, _cursor, parameters);
				if (_cursor< 2)
				{
					parameters.pop_back();
					parameters.pop_back();
				}
				script += parameters + ";\n";
			}
			else if (_mode == END_CLASS)
			{
				script += "\n\t}\n\tcatch (const std::exception&  ecept) {\n\n";
				script += "\t\tPRINT(\"Unit test (CLASS) constructor for " + _classname + " result by a crash\\n\", LogLvl::Error);\n";
				script += "\t\tPRINT(std::string(ecept.what())+\"\\n\", LogLvl::Error);\n";
				script += "\t}//end of trycatch\n\t}//end of if\n";
				script += "//end (CLASS)\n\n";

				_classname = "";
			}
			return "";
		}

		std::string SUnitTests::TryCatch(std::string _tryPart, std::string _catchPart) const
		{
			std::string out = "\ttry {\n";
			out += _tryPart;
			out += "\t}\n\tcatch (const std::exception&  except) {\n";
			if (_catchPart != "")
				out += _catchPart + "+ std::string(except.what())+\"\\n\";\n";
			out += "\t}\n";
			return out;
		}
#pragma	endregion CPPCode

#pragma	region HeaderCode
		void SUnitTests::FindHeaders()
		{
			std::ofstream file(UNIT_TEST_PATH + "UnitTestHeaders.list", ios::out | ios::trunc);
			file << "#include <Includes/ParadigmMath.h>" << std::endl;
			file << "#include <Core/Maths/FVector2.h>" << std::endl;
			file << "#include <Core/Maths/FVector3.h>" << std::endl;
			file << "#include <Core/Maths/FVector4.h>" << std::endl;
			file << "#include <Core/Maths/FMatrix4.h>" << std::endl;
			file << "#include <Core/Maths/FQuaternion.h>" << std::endl;
			file.close();
		}

		void SUnitTests::ParseHeaders()
		{
			std::ifstream file(UNIT_TEST_PATH + "UnitTestHeaders.list", ios::in);
			std::string line;
			while (file.good())
			{
				std::getline(file, line);
				includes += line + "\n";
				std::string str;
				bool ended = false, started = false;
				for (size_t i = 0; i < line.length() && !ended; ++i)
				{
					if (started)
						str.push_back(line[i]);
					else if (line[i] == '\"' || line[i] == '<' || line[i] == '>')
					{
						ended = started;
						started = !ended;
					}
				}
				if (!str.empty())
				{
					str.pop_back();
					ParseHeader(str);
				}
			}
		}

		void SUnitTests::ParseHeader(std::string path)
		{
			std::ifstream file(path, ios::in);
			if (!file)
				return;
			std::string Mod, Params, className;
			std::string line;
			bool classOpened = false;

			script += "\n\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t/* " + path + " */\n";
			script += "\t{\n\tstd::map<std::string, std::string> successList;\n\n";
			script += "  for(unsigned int i = 0; i < loopTimes ; ++i) {\n";
			while (file && file.good())
			{
				Mod.clear();
				Params.clear();
				std::getline(file, line);
				SUnitTests::EUnitTestMode modHandeler = SUnitTests::END_CLASS;
				for (unsigned int i = 0; !line.empty() && i < line.length(); ++i)
				{
					if (line[i] != ' ')
						Mod += line[i];
					if (Mod == "//") //if line is comented then skip it
						break;

					if (Mod == (std::string)"UNIT_TEST_NOCRASH")
					{
						ParseParams(line, i, Params);
						AddPieceOfCode(SUnitTests::NOCRASH, Params, className);
					}
					else if (Mod == (std::string)"UNIT_TEST_CRASH")
					{
						ParseParams(line, i, Params);
						AddPieceOfCode(SUnitTests::CRASH, Params, className);
					}
					else if (Mod == (std::string)"UNIT_TEST_COMPARE")
					{
						modHandeler = SUnitTests::COMPARE;
						for (unsigned int j = i+1; j < line.length() && line[j]!=' ' && line[j] != '('; ++j)
						{
							Mod += line[j];
							if (Mod == (std::string)"UNIT_TEST_COMPARE_scalarS")
							{
								modHandeler = SUnitTests::COMPARE_scalarS;
								i = j;
								break;
							}
						}
						ParseParams(line, i, Params);
						Mod.clear();
					}
					else if (Mod == (std::string)"UNIT_TEST_EXPECTEDRESULT" && modHandeler != SUnitTests::END_CLASS)
					{
						ParseParams(line, i, Params);
						AddPieceOfCode(modHandeler, Params, className);
						modHandeler = SUnitTests::END_CLASS;
					}
					else if (Mod == (std::string)"UNIT_TEST_ISTRUE")
					{
						ParseParams(line, i, Params);
						AddPieceOfCode(SUnitTests::ISTRUE, Params, className);
					}
					else if (Mod == (std::string)"UNIT_TEST_ISFALSE")
					{
						ParseParams(line, i, Params);
						AddPieceOfCode(SUnitTests::ISFALSE, Params, className);
					}
					else if (Mod == (std::string)"UNIT_TEST_CLASS")
					{
						ParseParams(line, i, Params);
						if (classOpened)
						{
							AddPieceOfCode(SUnitTests::END_CLASS, "", className);
							classOpened = false;
						}

						AddPieceOfCode(SUnitTests::CLASS, Params, className);
						classOpened = true;
					}
					else if (Mod == (std::string)"UNIT_TEST_END_CLASS")
					{
						AddPieceOfCode(SUnitTests::END_CLASS, "", className);
						classOpened = false;
					}
				}

				if (modHandeler != SUnitTests::END_CLASS)
					PRINT("UNIT_TEST_COMPARE or UNIT_TEST_COMPARE_scalar expect a UNIT_TEST_EXPECTEDRESULT right after it, on the same line. Can't find it on the line: " + line + "\n", EParadigmDebugLevel::ERROR_LEVEL);
			}

			if (classOpened)
				AddPieceOfCode(SUnitTests::END_CLASS, "", className);
			script += "\t}\n\n\n";

			script += "\n\tfor (auto i = successList.begin(); i != successList.end(); i++)	{\n";
			script += "\t\tif (i->second == \"\") {\n";
			script += "\t\t\tPRINT(i->first + \" OK.\\n\\t\", LogLvl::GREEN);\n\t\t}\n";
			script += "\t\telse {\n";
			script += "\t\t\tPRINT(i->first + \" FAIL :\\n\\t\" + i->second, LogLvl::Error)\n\t\t}\n\t}\n\t}\n";
			script += "\tstd::cout<<std::endl<<std::endl;\n";
		}

		void SUnitTests::ParseParams(std::string& line, unsigned int& index, std::string& _outParams)
		{
			int ParenteseCount = 0;
			bool started = false;
			std::string buff;
			for (; !line.empty() && index < line.length(); ++index)
			{
				if (line[index] == '(')
				{
					ParenteseCount++;
					started = true;
				}
				else if (line[index] == ')')
					ParenteseCount--;

				if (started)
				{
					if (line[index] == '\\' || line[index] == '\"')
						_outParams += "\\";
					_outParams += line[index];
					if (ParenteseCount == 0)
						break;
				}
			}
			if (ParenteseCount != 0)
				PRINT("Macro Parsing Failed, the line " + line + " never closed." + std::to_string(ParenteseCount), EParadigmDebugLevel::ERROR_LEVEL);
		}
#pragma	endregion HeaderCode
	}
}