#include "ontointerpreter.h"

namespace mbdev_ontology {

#ifdef DEBUG
bool ontointerpreter::debugMode = false;
#endif

ontointerpreter::ontointerpreter(ontology& onto)
   : onto(onto), keywords(), disallowed(), disallowedStarts() {
   initialize();
}

string ontointerpreter::execute(const string& command) {
   try {
      if(command.length() == 0)
         throw std::runtime_error("empty command");
      string_vector parsed = command.toVector(' ');
      parsed.trim().cleanse();
      size_t size = parsed.size();
      if(size == 0)
         throw std::runtime_error("command contains only whitespace characters");
      // execution of a specific 'root' command
      string& s = parsed[0];
      if(s.equals("add")) return executeAdd(parsed);
      if(s.equals("set")) return executeSet(parsed);
      if(s.equals("delete")) return executeDelete(parsed);
      //throw std::runtime_error("deletion not implemented");
      if(s.equals("find")) return executeFind(parsed);
      if(s.equals("load")) throw std::runtime_error("load/save not implemented");
      if(s.equals("save")) throw std::runtime_error("load/save not implemented");
      return executeStatement(parsed);
   } catch (std::runtime_error e) {
      std::stringstream ss;
      ss << "execution error for command '";
      ss << command;
      ss << "': ";
      ss << e.what();
      throw std::runtime_error(ss.str().c_str());
   }
}

string ontointerpreter::getClue(const string& current) {
   const string& s = current;
   std::stringstream ss;
   if(s.equals(""))
      ss << "add delete exit find load save set";
   else if(!s.compare("add")) {
      ss << "category";
      string_vector v = onto.getCategories();
      for(unsigned int i=0; i<v.size(); i++)
         ss << " " << v[i];
   } else if(s.equals("find")) {
      ss << "all";
   } else if(s.equals("find all")) {
      ss << "that";
   } else if(s.length()>=3 && !s.substr(0, 3).compare("set")) {
      string_vector cmd = s.toVector(' ');
      if(cmd.size() == 1) {
         string_vector v = onto.getElements();
         for(unsigned int i=0; i<v.size(); i++)
            ss << " " << v[i];
      } else if(cmd.size() >= 2) {
         element* e = onto.findElement(cmd[1]);
         if(e != nullptr) {
            if(cmd.size() == 2) {
               ss << "not transitive";
            } else if(cmd.size() == 3 && !cmd[2].compare("not")) {
               ss << "transitive";
            }
         }
      }
   } else {
   }
   return string(ss.str());
}

string_vector ontointerpreter::prepareDemo() {
   string demo_s;
   demo_s = "add category (thing,feature,situation);"\
         "add thing (animal, rabbit,bird ,wings,cat, mouse,eagle, penguin , eyes,  dog);"\
         "add thing (hawk,horse,pony,cheetah,tiger,lion,giraffe);"\
         "add thing  (  wheels   , engine, windows, aeroplane, object, building)  ;"\
         "add feature (big, large, small, black, transparent, loud);"\
         "add situation (is,has,eats,loves,likes,barks_at);"\
         "set is transitive;set loves not transitive;"\
         "rabbit is: animal;bird is: animal;small bird is: bird;"\
         "bird has: wings;aeroplane has: wings;aeroplane has: loud engine;"\
         "small bird loves: engine;large bird eats: rabbit;dog barks_at: cat;"\
         "cat eats: mouse;mouse likes: dog;eagle is: bird;"\
         "hawk is: bird;tiger is: large cat;lion is: large cat;"\
         "cat is: animal;dog is: animal;cheetah is: large cat;"\
         "large animal is: animal;small animal is: animal;building is: large object;"\
         "large cat is: large animal;large cat is: cat;"\
         "find all that has: wings;find all that is: animal"
         ;
   demo_s = "add category (o);"\
         "add o (aaa,bbb,ccc,ddd,eee,fff);"\
         "aaa: bbb;bbb: ccc;ccc: ddd;ddd: eee;eee: fff"
         ;
   return demo_s.toVector(';').trim().cleanse();
}

void ontointerpreter::initialize() {
#ifdef DEBUG
   if(debugMode) {
      onto.debugMode = true;
   }
#endif
   string keywords_s("add,set,delete,find,load,save,category,all,not,transitive,debug");
   keywords += keywords_s.toVector(',');
   string disallowed_s(", : ( ) . ; ! / \\ [ ] +");
   disallowed += disallowed_s.toVector(' ');
   string disallowedStarts_s("#,internal_");
   disallowedStarts += disallowedStarts_s.toVector(',');
}

string ontointerpreter::executeAdd(string_vector& arguments) {
   size_t size = arguments.size();
   if(size < 3)
      throw std::runtime_error("classes(s) and name(s) of added object(s) must be provided");
   string& first = arguments[2];
   string& last = arguments[size-1];
   // adding multiple categories/elements
   if(first.at(0) == '(' && last.at(last.length()-1)) {
      // erase '(' and ')'
      first.erase(first.begin());
      last.erase(last.end() - 1);
      arguments.cleanse();
      size = arguments.size(); // after cleansing
      //erase extra ','
      for(size_t i = 2; i < size; i++) {
         size_t len = arguments[i].length();
         if(len == 0)
            continue;
         if(arguments[i].at(len - 1) == ',') {
            arguments[i].erase(arguments[i].end() - 1);
            if(len == 1)
               continue;
         }
         if(arguments[i].at(0) == ',')
            arguments[i].erase(arguments[i].begin());
      }
      arguments.cleanse();
      string s = arguments.str((const char)',', 2);
      string_vector vec = s.toVector(',');
      if(arguments[1].equals("category"))
         return onto.addCategories(vec);
      return onto.addElements(arguments[1], vec);
   }
   //adding a single category/element
   if(arguments[1].equals("category"))
      return onto.addCategory(arguments[2]);
   return onto.addElement(arguments[1], arguments[2]);
}

string ontointerpreter::executeSet(string_vector& arguments) {
   size_t size = arguments.size();
   if(size < 3)
      throw std::runtime_error("too few arguments");
   //string& s = parameters[1];
   element* e = onto.findElement(arguments[1]);
   if(e == nullptr)
      throw std::runtime_error("cannot set parameter of a non-existing element");
   bool positive = true;
   string* ptr = &(arguments[2]);
   // will the parameter be set to 'true' or 'false'?
   // and what is the parameter's name?
   if(size == 4 && arguments[2].equals("not")) {
      positive = false;
      ptr = &(arguments[3]);
   } else if(size !=3) {
      throw std::runtime_error("correct syntax is \"set <element_name> [not] <parameter_name>\"");
   }
   string& paramName = *ptr;
   std::stringstream result;
   result << "element's ";
   if(paramName.equals("transitive")) {
      result << "transitivity";
      e->setTransitive(positive);
   } else {
      std::stringstream s;
      s << "there is no such parameter: ";
      s << paramName;
      throw std::runtime_error(s.str().c_str());
   }
   result << " is now ";
   result << (positive ? "enabled" : "disabled");
   return string(result.str());
}

string ontointerpreter::executeDelete(string_vector& arguments) {
   size_t size = arguments.size();
   if(size < 3)
      throw std::runtime_error("classes(s) and name(s) of deleted object(s) must be provided");
   string& first = arguments[2];
   string& last = arguments[size-1];
   // adding multiple categories/elements
   if(first.at(0) == '(' && last.at(last.length()-1)) {
      // erase '(' and ')'
      first.erase(first.begin());
      last.erase(last.end() - 1);
      arguments.cleanse();
      size = arguments.size(); // after cleansing
      // ...
      throw std::runtime_error("not implemented: deletion of multiple elems/cats");
   }
   //deleting a single category/element
   if(arguments[1].equals("category"))
      return onto.deleteCategory(arguments[2]);
   return onto.deleteElement(/*arguments[1], */arguments[2]);
}

string ontointerpreter::executeFind(string_vector& arguments) {
   size_t size = arguments.size();
   if(size < 2)
      throw std::runtime_error("'find' command must have parameters");
   string& what = arguments[1];
   //category* cat = nullptr;
   if(!what.equals("all")) {
      throw std::runtime_error("correct syntax is: find all [that <statement>]");
      //cat = onto.findCategory(what);
      //if(cat == nullptr)
      //   throw std::runtime_error("cannot find the element from a non-existing category");
   }
   if(size == 2) {
      if(what.equals("all"))
         return onto.toString();
      throw std::runtime_error("'find <category_name>' is not implemented");
   }
   //string& s2 = arguments[2];
   if(!arguments[2].equals("that")) {
      throw std::runtime_error("missing keyword 'that'");
   }
   if(size < 4)
      throw std::runtime_error("too few arguments");
   int index = arguments.indexContains(":");
   if(index < 0)
      throw std::runtime_error("searched statement must have exactly two parts");

   //   std::stringstream ss;
   //   for(unsigned int i=3; i<size; i++)
   //      ss << parameters[i] << ' ';
   //   string str = string(ss.str());
   //   str.trim();

   // split the statement into left and right parts

   //   string_vector vec = str.toVector(':');
   //   Strings::trimVector(vec);
   //   if(vec.size() != 2)
   //      throw std::runtime_error("searched statement must have exactly two parts");

   // ... left hand side
   string str = arguments.str(' ', 3, index + 1 - 3).trim();
   if(str.length() == 0)
      throw std::runtime_error("left side of statement is empty");
   str.erase(str.end() - 1);
   string_vector vecL = str.toVector(' ').trim().cleanse();
   cell left(vecL, onto);
   // .. and right hand side
   string_vector vecR = arguments.str(' ', index + 1).toVector(' ').trim().cleanse();
   cell right(vecR, onto);
   // looking for all matching statements
   statement s(&left, &right);
   ptr_vector<statement> sv = onto.findStatements(s);
   // combining results into one string
   std::stringstream results;
#ifdef DEBUG
   if(debugMode) {
      std::cout << "Cells before: " << onto.getCells().toString() << "\n";
      std::cout.flush();
   }
#endif
   size_t sizeSt = sv.size();
   for(size_t n=0; n<sizeSt; n++) {
      cell *c = sv[n]->first;
      cell copy(*c);
      for(size_t i=0; i<vecL.size(); i++) {
         //it = copy.find(findCategory(vecL[0]));
         element* e = onto.findElement(vecL[i]);
         copy.erase(e->getCategoryPtr());
      }
      cell::iterator it;
      for (it = copy.begin(); it != copy.end(); ++it)
         results << ' ' << *(it->second);
      if(n < sizeSt - 1)
         results << ",";
   }
#ifdef DEBUG
   if(debugMode) {
      std::cout << "Cells after:  " << onto.getCells().toString() << "\n";
      std::cout.flush();
   }
#endif
   return string(results.str());
}

string ontointerpreter::executeStatement(string_vector& arguments) {
   //handling of natural language
   int index = arguments.indexContains(":");
   if(index < 0)
      throw std::runtime_error("colon (':') not found in the statement. \"a: b\" == \"a implies b\"");
   string s1 = arguments.str(' ', 0, index + 1).trim();
   s1.erase(s1.end() - 1);
   string s2 = arguments.str(' ', index + 1).trim();
   string_vector leftVec = s1.toVector(' ').trim();
   cell* left = new cell(leftVec, onto);
   string_vector rightVec = s2.toVector(' ').trim();
   cell* right = new cell(rightVec, onto);
   return onto.addStatement(*left, *right);
}

}
