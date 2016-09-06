#string
static const size_t npos = -1;

##string
Constructs a string object

###API
string();

string (const string& str);

string (const string& str, size_t pos, size_t len = npos);

string (const char* s);

string (const char* s, size_t n);

string (size_t n, char c);

template <class InputIterator>
  string  (InputIterator first, InputIterator last);


###parameters
- str: Another string object, whose value is either copied or acquired.
- pos: Position of the first character in str that is copied to the object as a substring.
- len: Length of the substring to be copied (if the string is shorter, as many characters as possible are copied).
- s: Pointer to an array of characters (such as a c-string).
- n: Number of characters to copy.
- first, last: Input iterators to the initial and final positions in a range. The range used is [first,last), which includes all the characters between first and last, including the character pointed by first but not the character pointed by last.
- il: An initializer_list object.


###example
```cpp
// string constructor
#include <iostream>
#include <string>

int main ()
{
  std::string s0 ("Initial string");

  // constructors used in the same order as described above:
  std::string s1;
  std::string s2 (s0);
  std::string s3 (s0, 8, 3);
  std::string s4 ("A character sequence", 6);
  std::string s5 ("Another character sequence");
  std::string s6a (10, 'x');
  std::string s6b (10, 42);      // 42 is the ASCII code for '*'
  std::string s7 (s0.begin(), s0.begin()+7);

  std::cout << "s1: " << s1 << "\ns2: " << s2 << "\ns3: " << s3;
  std::cout << "\ns4: " << s4 << "\ns5: " << s5 << "\ns6a: " << s6a;
  std::cout << "\ns6b: " << s6b << "\ns7: " << s7 << '\n';
  return 0;
}
```
```
s1: 
s2: Initial string
s3: str
s4: A char
s5: Another character sequence
s6a: xxxxxxxxxx
s6b: **********
s7: Initial
```

##begin
Returns an iterator pointing to the first character of the string.

###API
iterator begin();

const_iterator begin() const;

###parameters
none

##return
An iterator to the beginning of the string.

###example
```cpp
// string::begin/end
#include <iostream>
#include <string>

int main ()
{
  std::string str ("Test string");
  for ( std::string::iterator it=str.begin(); it!=str.end(); ++it)
    std::cout << *it;
  std::cout << '\n';

  return 0;
}
```
```
Test string
```

##compare
Compares the value of the string object (or a substring) to the sequence of characters specified by its arguments.

###API
int compare (const string& str) const;

int compare (size_t pos, size_t len, const string& str) const;

int compare (size_t pos, size_t len, const string& str,
             size_t subpos, size_t sublen) const;

int compare (const char* s) const;

int compare (size_t pos, size_t len, const char* s) const;

int compare (size_t pos, size_t len, const char* s, size_t n) const;

###parameters
- str: Another string object, used entirely (or partially) as the comparing string.
- pos: Position of the first character in the compared string.
- len: Length of compared string (if the string is shorter, as many characters as possible).
- subpos, sublen: Same as pos and len above, but for the comparing string.
- s: Pointer to an array of characters.
- n: Number of characters to compare.

##return
Returns a signed integral indicating the relation between the strings:

|value	|relation between compared string and comparing string|
| ------------- |-------------|
|0	|They compare equal|
|<0	|Either the value of the first character that does not match is lower in the compared string, or all compared characters match but the compared string is shorter.|
|>0	|Either the value of the first character that does not match is greater in the compared string, or all compared characters match but the compared string is longer.|

###example
```cpp
// comparing apples with apples
#include <iostream>
#include <string>

int main ()
{
  std::string str1 ("green apple");
  std::string str2 ("red apple");

  if (str1.compare(str2) != 0)
    std::cout << str1 << " is not " << str2 << '\n';

  if (str1.compare(6,5,"apple") == 0)
    std::cout << "still, " << str1 << " is an apple\n";

  if (str2.compare(str2.size()-5,5,"apple") == 0)
    std::cout << "and " << str2 << " is also an apple\n";

  if (str1.compare(6,5,str2,4,5) == 0)
    std::cout << "therefore, both are apples\n";

  return 0;
}
```
```
green apple is not red apple
still, green apple is an apple
and red apple is also an apple
therefore, both are apples
```


##c_str
Returns a pointer to an array that contains a null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.

###API
const char* c_str() const;

###parameters
none

##return
A pointer to the c-string representation of the string object's value.

###example
```cpp
// strings and c-strings
#include <iostream>
#include <cstring>
#include <string>

int main ()
{
  std::string str ("Please split this sentence into tokens");

  char * cstr = new char [str.length()+1];
  std::strcpy (cstr, str.c_str());

  // cstr now contains a c-string copy of str

  char * p = std::strtok (cstr," ");
  while (p!=0)
  {
    std::cout << p << '\n';
    p = std::strtok(NULL," ");
  }

  delete[] cstr;
  return 0;
}
```
```
Please
split
this
sentence
into
tokens
```

##erase
Erases part of the string, reducing its length:

###API
string& erase (size_t pos = 0, size_t len = npos);
	
iterator erase (iterator p);

iterator erase (iterator first, iterator last);

###parameters
- pos: Position of the first character to be erased.
- len: Number of characters to erase (if the string is shorter, as many characters as possible are erased).
- p: Iterator to the character to be removed.
- first, last: Iterators specifying a range within the string] to be removed: [first,last). i.e., the range includes all the characters between first and last, including the character pointed by first but not the one pointed by last.

##return
The sequence version (1) returns *this.
The others return an iterator referring to the character that now occupies the position of the first character erased, or string::end if no such character exists.

###example
```cpp
// string::erase
#include <iostream>
#include <string>

int main ()
{
  std::string str ("This is an example sentence.");
  std::cout << str << '\n';
                                           // "This is an example sentence."
  str.erase (10,8);                        //            ^^^^^^^^
  std::cout << str << '\n';
                                           // "This is an sentence."
  str.erase (str.begin()+9);               //           ^
  std::cout << str << '\n';
                                           // "This is a sentence."
  str.erase (str.begin()+5, str.end()-9);  //       ^^^^^
  std::cout << str << '\n';
                                           // "This sentence."
  return 0;
}
```
```
This is an example sentence.
This is an sentence.
This is a sentence.
This sentence.
```


##find
Searches the string for the first occurrence of the sequence specified by its arguments.

###API
size_t find (const string& str, size_t pos = 0) const;
	
size_t find (const char* s, size_t pos = 0) const;

size_t find (const char* s, size_t pos, size_t n) const;

size_t find (char c, size_t pos = 0) const;

###parameters
- str: Another string with the subject to search for.
- pos: Position of the first character in the string to be considered in the search.
- s: Pointer to an array of characters.
- n: Length of sequence of characters to match.
- c: Individual character to be searched for.

##return
The position of the first character of the first match.
If no matches were found, the function returns string::npos.

###example
```cpp
// string::find
#include <iostream>       // std::cout
#include <string>         // std::string

int main ()
{
  std::string str ("There are two needles in this haystack with needles.");
  std::string str2 ("needle");

  // different member versions of find in the same order as above:
  std::size_t found = str.find(str2);
  if (found!=std::string::npos)
    std::cout << "first 'needle' found at: " << found << '\n';

  found=str.find("needles are small",found+1,6);
  if (found!=std::string::npos)
    std::cout << "second 'needle' found at: " << found << '\n';

  found=str.find("haystack");
  if (found!=std::string::npos)
    std::cout << "'haystack' also found at: " << found << '\n';

  found=str.find('.');
  if (found!=std::string::npos)
    std::cout << "Period found at: " << found << '\n';

  // let's replace the first needle:
  str.replace(str.find(str2),str2.length(),"preposition");
  std::cout << str << '\n';

  return 0;
}
```
```
first 'needle' found at: 14
second 'needle' found at: 44
'haystack' also found at: 30
Period found at: 51
There are two prepositions in this haystack with needles.
```

##find\_first_of
Searches the string for the first character that matches **any** of the characters specified in its arguments.

###API	
size_t find_first_of (const string& str, size_t pos = 0) const;

size_t find_first_of (const char* s, size_t pos = 0) const;

size_t find_first_of (const char* s, size_t pos, size_t n) const;

size_t find_first_of (char c, size_t pos = 0) const;

###parameters
- str: Another string with the characters to search for.
- pos: Position of the first character in the string to be considered in the search.
- s: Pointer to an array of characters.
- n: Number of character values to search for.
- c: Individual character to be searched for.

##return
The position of the first character that matches.
If no matches are found, the function returns string::npos.

###example
```cpp
// string::find_first_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t

int main ()
{
  std::string str ("Please, replace the vowels in this sentence by asterisks.");
  std::size_t found = str.find_first_of("aeiou");
  while (found!=std::string::npos)
  {
    str[found]='*';
    found=str.find_first_of("aeiou",found+1);
  }

  std::cout << str << '\n';

  return 0;
}
```
```
Pl**s*, r*pl*c* th* v*w*ls *n th*s s*nt*nc* by *st*r*sks.
```

##find\_first\_not_of
Searches the string for the first character that does not match **any** of the characters specified in its arguments.

###API
同find_first_of

###parameters
同find_first_of

###return
The position of the first character that does not match.
If no such characters are found, the function returns string::np

###example
```cpp
// string::find_first_not_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t

int main ()
{
  std::string str ("look for non-alphabetic characters...");

  std::size_t found = str.find_first_not_of("abcdefghijklmnopqrstuvwxyz ");

  if (found!=std::string::npos)
  {
    std::cout << "The first non-alphabetic character is " << str[found];
    std::cout << " at position " << found << '\n';
  }

  return 0;
}
```
```
The first non-alphabetic character is - at position 12
```

##find\_last_of
Searches the string for the last character that matches any of the characters specified in its arguments.

###API
同find_first_of

###parameters
同find_first_of

##return
The position of the last character that matches.
If no matches are found, the function returns string::npos.

###example
```cpp
// string::find_last_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>         // std::size_t

void SplitFilename (const std::string& str)
{
  std::cout << "Splitting: " << str << '\n';
  std::size_t found = str.find_last_of("/\\");
  std::cout << " path: " << str.substr(0,found) << '\n';
  std::cout << " file: " << str.substr(found+1) << '\n';
}

int main ()
{
  std::string str1 ("/usr/bin/man");
  std::string str2 ("c:\\windows\\winhelp.exe");

  SplitFilename (str1);
  SplitFilename (str2);

  return 0;
}
```
```
Splitting: /usr/bin/man
 path: /usr/bin
 file: man
Splitting: c:\windows\winhelp.exe
 path: c:\windows
 file: winhelp.exe
```

##find\_last\_not_of
Searches the string for the last character that does not match any of the characters specified in its arguments.

###API
同find_first_of

###parameters
同find_first_of

##return
The position of the first character that does not match.
If no such characters are found, the function returns string::npos.

###example
```cpp
// string::find_last_not_of
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t

int main ()
{
  std::string str ("Please, erase trailing white-spaces   \n");
  std::string whitespaces (" \t\f\v\n\r");

  std::size_t found = str.find_last_not_of(whitespaces);
  if (found!=std::string::npos)
    str.erase(found+1);
  else
    str.clear();            // str is all whitespace

  std::cout << '[' << str << "]\n";

  return 0;
}
```
```
[Please, erase trailing white-spaces]
```

##insert
Inserts additional characters into the string right before the character indicated by pos (or p):

###API
string& insert (size_t pos, const string& str);

string& insert (size_t pos, const string& str, size_t subpos, size_t sublen);
	
string& insert (size_t pos, const char* s);

string& insert (size_t pos, const char* s, size_t n);

string& insert (size_t pos, size_t n, char c);

void insert (iterator p, size_t n, char c);
	
iterator insert (iterator p, char c);

template <class InputIterator>
   void insert (iterator p, InputIterator first, InputIterator last);

###parameters
- pos: Insertion point: The new contents are inserted before the character at position pos.
- str: Another string object.
- subpos: Position of the first character in str that is inserted into the object as a substring.
- sublen: Length of the substring to be copied (if the string is shorter, as many characters as possible are copied).
- s: Pointer to an array of characters (such as a c-string).
- n: Number of characters to insert.
- c: Character value.
- p: Iterator pointing to the insertion point: The new contents are inserted before the character pointed by p.
- first, last: Input iterators to the initial and final positions in a range. The range used is [first,last), which includes all the characters between first and last, including the character pointed by first but not the character pointed by last.
- il: An initializer_list object.

##return
The signatures returning a reference to string, return *this.
Those returning an iterator, return an iterator pointing to the first character inserted.

###example
```cpp
// inserting into a string
#include <iostream>
#include <string>

int main ()
{
  std::string str="to be question";
  std::string str2="the ";
  std::string str3="or not to be";
  std::string::iterator it;

  // used in the same order as described above:
  str.insert(6,str2);                 // to be (the )question
  str.insert(6,str3,3,4);             // to be (not )the question
  str.insert(10,"that is cool",8);    // to be not (that is )the question
  str.insert(10,"to be ");            // to be not (to be )that is the question
  str.insert(15,1,':');               // to be not to be(:) that is the question
  it = str.insert(str.begin()+5,','); // to be(,) not to be: that is the question
  str.insert (str.end(),3,'.');       // to be, not to be: that is the question(...)
  str.insert (it+2,str3.begin(),str3.begin()+3); // (or )

  std::cout << str << '\n';
  return 0;
}
```
```
to be, or not to be: that is the question...
```


##length
Returns the length of the string, in terms of bytes.

###API
size_t length() const;

###parameters
none

###return
The number of bytes in the string.

###example
```cpp
// string::length
#include <iostream>
#include <string>

int main ()
{
  std::string str ("Test string");
  std::cout << "The size of str is " << str.length() << " bytes.\n";
  return 0;
}
```
```
The size of str is 11 bytes
```

##replace
Replaces the portion of the string that begins at character pos and spans len characters (or the part of the string in the range between [i1,i2)) by new contents:

###API
string& replace (size_t pos,  size_t len,  const string& str);

string& replace (iterator i1, iterator i2, const string& str);

string& replace (size_t pos,  size_t len,  const string& str,
                 size_t subpos, size_t sublen);

string& replace (size_t pos,  size_t len,  const char* s);

string& replace (iterator i1, iterator i2, const char* s);
	
string& replace (size_t pos,  size_t len,  const char* s, size_t n);

string& replace (iterator i1, iterator i2, const char* s, size_t n);

string& replace (size_t pos,  size_t len,  size_t n, char c);

string& replace (iterator i1, iterator i2, size_t n, char c);

template <class InputIterator>
  string& replace (iterator i1, iterator i2,
                   InputIterator first, InputIterator last);

###parameters
- str: Another string object, whose value is copied.
- pos: Position of the first character to be replaced.
- len: Number of characters to replace (if the string is shorter, as many characters as possible are replaced).
- subpos: Position of the first character in str that is copied to the object as replacement.
- sublen: Length of the substring to be copied (if the string is shorter, as many characters as possible are copied).
- s: Pointer to an array of characters (such as a c-string).
- n: Number of characters to copy.
- c: Character value, repeated n times.
- first, last: Input iterators to the initial and final positions in a range. The range used is [first,last), which includes all the characters between first and last, including the character pointed by first but not the character pointed by last.
- il: An initializer_list object.

###return
*this

###example
```cpp
// replacing in a string
#include <iostream>
#include <string>

int main ()
{
  std::string base="this is a test string.";
  std::string str2="n example";
  std::string str3="sample phrase";
  std::string str4="useful.";

  // replace signatures used in the same order as described above:

  // Using positions:                 0123456789*123456789*12345
  std::string str=base;           // "this is a test string."
  str.replace(9,5,str2);          // "this is an example string." (1)
  str.replace(19,6,str3,7,6);     // "this is an example phrase." (2)
  str.replace(8,10,"just a");     // "this is just a phrase."     (3)
  str.replace(8,6,"a shorty",7);  // "this is a short phrase."    (4)
  str.replace(22,1,3,'!');        // "this is a short phrase!!!"  (5)

  // Using iterators:                                               0123456789*123456789*
  str.replace(str.begin(),str.end()-3,str3);                    // "sample phrase!!!"      (1)
  str.replace(str.begin(),str.begin()+6,"replace");             // "replace phrase!!!"     (3)
  str.replace(str.begin()+8,str.begin()+14,"is coolness",7);    // "replace is cool!!!"    (4)
  str.replace(str.begin()+12,str.end()-4,4,'o');                // "replace is cooool!!!"  (5)
  str.replace(str.begin()+11,str.end(),str4.begin(),str4.end());// "replace is useful."    (6)
  std::cout << str << '\n';
  return 0;
}
```
```
replace is useful.
```

##substr
Returns a newly constructed string object with its value initialized to a copy of a substring of this object.

###API
string substr (size_t pos = 0, size_t len = npos) const;

###parameters
- pos: Position of the first character to be copied as a substring.
- len: Number of characters to include in the substring (if the string is shorter, as many characters as possible are used).

###return
A string object with a substring of this object.

###example
```cpp
// string::substr
#include <iostream>
#include <string>

int main ()
{
  std::string str="We think in generalities, but we live in details.";
                                           // (quoting Alfred N. Whitehead)

  std::string str2 = str.substr (3,5);     // "think"

  std::size_t pos = str.find("live");      // position of "live" in str

  std::string str3 = str.substr (pos);     // get from "live" to the end

  std::cout << str2 << ' ' << str3 << '\n';

  return 0;
}
```
```
think live in details.
```

