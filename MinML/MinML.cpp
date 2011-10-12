//
//  MinML
//
//  Derivative work of the Java version by John Wilson.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "MinML.h"

//

#define kInitialBufferSize  256
#define kBufferIncrement    128

//

using namespace MinML;

//
//  Handler
//

void Handler::StartDocument(void)
{
}

void Handler::StartElement(const char *name, const AttributeList& attrs)
{
}

void Handler::EndElement(const char *name)
{
}

void Handler::Characters(char *ch)
{
}

void Handler::EndDocument(void)
{
}

void Handler::FatalError(const char *err, int line, int col)
{
}

//
//  AttributeList
//

AttributeList::AttributeList( void )
    : size(0), attributeNames(NULL), attributeValues(NULL)
{
}

AttributeList::~AttributeList( void )
{
    if (size)
        Flush();
}

void AttributeList::SaveName(const char *name)
{
    size += 1;
    attributeNames = (char **)realloc(attributeNames, size * sizeof(char *));
    attributeNames[size-1] = 0;
    attributeValues = (char **)realloc(attributeValues, size * sizeof(char *));
    attributeValues[size-1] = 0;

    attributeNames[size-1] = (char *)malloc(strlen(name)+1);
    strcpy(attributeNames[size-1], name);
}

void AttributeList::SaveValue(const char *value)
{
    attributeValues[size-1] = (char *)malloc(strlen(value)+1);
    strcpy(attributeValues[size-1], value);
}

void AttributeList::Flush(void)
{
    if (!size)
        return;

    for (int i = 0; i < size; i++) {
        if (attributeNames[i])
            free(attributeNames[i]);
        if (attributeValues[i])
            free(attributeValues[i]);
    }
    free(attributeNames);
    free(attributeValues);

    size = 0;
    attributeNames = NULL;
    attributeValues = NULL;
}

const char* AttributeList::getValue(const char* name) const
{
    for (int i = 0; i < size; i++)
        if (strcmp(attributeNames[i],name) == 0)
            return attributeValues[i];
    return 0;
}

//
//  Stack
//

Stack::Stack(void)
{
    buffer = (char *)malloc(kInitialBufferSize);
    size = kInitialBufferSize;

    top = buffer;
}

Stack::~Stack(void)
{
    free(buffer);
}

void Stack::Push(const char *name)
{
    int len = strlen(name) + 1;

    if ((top + len + 1) > (buffer + size)) {
        int offset = top - buffer;

        size += kBufferIncrement;
        buffer = (char *)realloc(buffer, size);
        top = buffer + offset;
    }

    strcpy(top, name);
    top += len;
    *top = len;
    top += 1;
}

const char* Stack::Pop(void)
{
    if (top == buffer) return 0;
    top -= 1;
    top -= *top;

    return top;
}

const char* Stack::Peek(void)
{
    char    *rslt
    ;

    if (top == buffer) return 0;
    rslt = top - 1;
    rslt -= *rslt;

    return rslt;
}

//
//  Buffer
//

Buffer::Buffer(HandlerPtr &hp)
    : handler(hp)
{
    buffer = (char *)malloc(kInitialBufferSize);
    size = kInitialBufferSize;

    end = buffer;
    *end  = 0;
}

Buffer::~Buffer(void)
{
    free(buffer);
}

void Buffer::Write(const char c)
{
    if ((end + 1) > (buffer + size)) {
        int offset = end - buffer;

        size += kBufferIncrement;
        buffer = (char *)realloc(buffer, size);
        end = buffer + offset;
    }

    *end++ = c;
}

void Buffer::Flush(void)
{
    *end = 0;
    handler->Characters(buffer);
    end = buffer;
}

const char* Buffer::GetString(void)
{
    *end = 0;
    return buffer;
}

//
//  Parser::Parser
//

Parser::Parser( void )
    : currentHandler(this), buffer(currentHandler)
{
}

//
//  Parser::~Parser
//

Parser::~Parser(void)
{
}

//
//  Parser::SetHandler
//

void Parser::SetHandler( HandlerPtr hp )
{
    currentHandler = hp;
}

//
//  Parser::Read
//

char Parser::Read(void)
{
    char c
    ;

    for (;;) {
        c = *currentString++;

        if (!c) {
            currentString = Reader();
            if (!currentString) {
                c = -1;
                break;
            }
        } else
            break;
    }

    return c;
}

//
//  Parser::Digit
//

int Parser::Digit(char ch, int radix)
{
    int     digit
    ;
    
    if (isdigit(ch)) {
        digit = (ch - '0');
        if (digit >= radix)
            digit = -1;
    } else
    if ((radix == 16) && (ch >= 'A') && (ch <= 'F'))
        digit = (ch - 'A') + 10;
    else
    if ((radix == 16) && (ch >= 'a') && (ch <= 'f'))
        digit = (ch - 'a') + 10;
    else
        digit = -1;
    
    return digit;
}

//
//  Parser::Parse
//

void Parser::Parse( void )
{
    int     charCount = 0
    ,       level = 0
    ,       mixedContentLevel = -1
    ;
    char    currentChar = 0
    ;
    const char *elementName = 0
    ;
    unsigned short  *state = operands[inSkipping]
    ,               transition
    ;
    
    lineNumber = 1;
    columnNumber = 0;
    
    buffer.Reset();
    tags.Flush();
    
    currentString = Reader();
    if (!currentString) return;
    
    for (;;) {
        charCount++;
        currentChar = (*currentString ? *currentString++ : Read());
        
        if (currentChar > ']') {
            transition = state[14];
        } else {
            int charClass = charClasses[currentChar + 1];
            
            if (charClass == -1) {
                currentHandler->FatalError("document contains illegal control character", lineNumber, columnNumber);
                return;
            }

            if (charClass == 12) {
                if (currentChar == '\r') {
                    currentChar = '\n';
                    charCount = -1;
                }

                if (currentChar == '\n') {
                    if (charCount == 0)
                        continue;  // preceeded by '\r' so ignore

                    if (charCount != -1)
                        charCount = 0;

                    lineNumber++;
                    columnNumber = 0;
                }
            }

            transition = state[charClass];
        }

        columnNumber++;

        unsigned short *operand = operands[transition >> 8];

        switch (transition & 0xFF) {
            case endStartName:
                // end of start element name
                elementName = buffer.GetString();
                
                if (tags.Empty())
                    currentHandler->StartDocument();

                tags.Push(elementName);
                buffer.Reset();
                
                if ((currentChar != '>') && (currentChar != '/'))
                    break;  // change state to operand
                
                // drop through to emit start element (we have no attributes)

            case emitStartElement:
                // emit start element
                StartElement( tags.Peek(), attrs );
                
                attrs.Flush();

                if (mixedContentLevel != -1) mixedContentLevel++;

                if (currentChar != '/')
                    break;  // change state to operand

                // <element/> drop through

            case emitEndElement:
                // emit end element

                if (tags.Empty()) {
                    currentHandler->FatalError("end tag at begining of document", lineNumber, columnNumber);
                    return;
                }
                
                {
                    const char* begin = tags.Pop();

                    elementName = buffer.GetString();
                    if ((currentChar != '/') && (strcmp(elementName,begin) != 0)) {
                        currentHandler->FatalError( "end tag does not match begin tag", lineNumber, columnNumber );
                        return;
                    } else {
                        currentHandler->EndElement(begin);

                        if (tags.Empty()) {
                            currentHandler->EndDocument();
                            return;
                        }
                    }
                    
                    buffer.Reset();
                }

                if (mixedContentLevel != -1) --mixedContentLevel;

                break;  // change state to operand

            case emitCharacters:
                // emit characters

                buffer.Flush();
                break;  // change state to operand

            case emitCharactersSave:
                // emit characters and save current character

                if (mixedContentLevel == -1) mixedContentLevel = 0;

                buffer.Flush();

                buffer.Write((char)currentChar);

                break;  // change state to operand

            case possiblyEmitCharacters:
                // write any skipped whitespace if in mixed content

                if (mixedContentLevel != -1) buffer.Flush();
                break;  // change state to operand

            case saveAttributeName:
                // save attribute name

                attrs.SaveName(buffer.GetString());
                buffer.Reset();
                
                break;  // change state to operand

            case saveAttributeValue:
                // save attribute value

                attrs.SaveValue(buffer.GetString());
                buffer.Reset();
                
                break;  // change state to operand

            case startComment:
                // change state if we have found "<!--"

                if (Read() != '-') continue; // not "<!--"

                break;  // change state to operand

            case endComment:
                // change state if we find "-->"

                if ((currentChar = Read()) == '-') {
                    // deal with the case where we might have "------->"
                    while ((currentChar = Read()) == '-');

                    if (currentChar == '>') break;  // end of comment, change state to operand
                }

                continue;   // not end of comment, don't change state

            case incLevel:
                level++;
                break;

            case decLevel:
                if (level == 0)
                    break; // outer level <> change state
                level--;
                continue; // in nested <>, don't change state

            case startCDATA:
                // change state if we have found "<![CDATA["
                if (Read() != 'C') continue;   // don't change state
                if (Read() != 'D') continue;   // don't change state
                if (Read() != 'A') continue;   // don't change state
                if (Read() != 'T') continue;   // don't change state
                if (Read() != 'A') continue;   // don't change state
                if (Read() != '[') continue;   // don't change state
                break;  // change state to operand

            case endCDATA:
                // change state if we find "]]>"

                if ((currentChar = Read()) == ']') {
                    // deal with the case where we might have "]]]]]]]>"
                    while ((currentChar = Read()) == ']') buffer.Write(']');

                    if (currentChar == '>') break;  // end of CDATA section, change state to operand

                    buffer.Write(']');
                }

                buffer.Write(']');
                buffer.Write(currentChar);
                continue;   // not end of CDATA section, don't change state

            case processCharRef:
                // process character entity

                {
                    int crefState = 0;
                    static const char* crefString = "#amp;&pos;'quot;\"gt;>lt;<";
                    static const char* crefNextState
                                            = "\x01\x0b\x06\xff\xff\xff\xff\xff\xff\xff\xff"
                                            //   #   a   m   p   ;   &   p   o   s   ;   '
                                            //   0   1   2   3   4   5   6   7   8   9   a
                                            "\x11\xff\xff\xff\xff\xff\x15\xff\xff\xff"
                                            //   q   u   o   t   ;   "   g   t   ;   >
                                            //   b   b   d   e   f   10  11  12  13  14
                                            "\xff\xff\xff";
                                            //   l   t   ;
                                            //   15  16  17

                    currentChar = Read();

                    for (;;) {
                        if (crefString[crefState] == currentChar) {
                            crefState++;

                            if (currentChar == ';') {
                                buffer.Write(crefString[crefState]);
                                break;
                            } else
                            if (currentChar == '#') {
                                int radix;

                                currentChar = Read();

                                if (currentChar == 'x') {
                                    radix = 16;
                                    currentChar = Read();
                                } else {
                                    radix = 10;
                                }

                                int charRef = Digit(currentChar, radix);

                                for (;;) {
                                    currentChar = Read();

                                    int digit = Digit(currentChar, radix);

                                    if (digit == -1) break;

                                    charRef = (char)((charRef * radix) + digit);
                                }

                                if (currentChar == ';' && charRef != -1) {
                                    if (charRef > 255) {
                                        currentHandler->FatalError("invalid Character Entity", lineNumber, columnNumber);
                                        return;
                                    }
                                    buffer.Write(charRef);
                                    break;
                                }

                                currentHandler->FatalError("invalid Character Entity", lineNumber, columnNumber);
                                return;
                            } else
                                currentChar = Read();
                        } else {
                            crefState = crefNextState[crefState];
                            if (crefState == '\xff') {
                                currentHandler->FatalError( "invalid Character Entity", lineNumber, columnNumber);
                                return;
                            }
                        }
                    }
                }
                break;

            case parseError:
                // report fatal error
                currentHandler->FatalError(errorStr[transition >> 8], lineNumber, columnNumber);
                // drop through to exit parser

            case exitParser:
                // exit parser
                return;

            case writeCdata:
                // write character data
                // this will also write any skipped whitespace

                buffer.Write(currentChar);
                break;  // change state to operand

            case discardAndChange:
                // throw saved characters away and change state

                buffer.Reset();
                break;  // change state to operand

            case discardSaveAndChange:
                // throw saved characters away, save character and change state

                buffer.Reset();
                // drop through to save character and change state

            case saveAndChange:
                // save character and change state

                buffer.Write(currentChar);
                break;  // change state to operand

            case change:
                // change state to operand
                break;  // change state to operand
        }

        state = operand;
    }
}

//
//  Character Classes
//

char Parser::charClasses[] =
    {
    //  EOF
        13,
    //                                      \t  \n          \r
        -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 12, -1, -1, 12, -1, -1,
    //
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    //  SP   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
        12,  8,  7, 14, 14, 14,  3,  6, 14, 14, 14, 14, 14, 11, 14,  2,
    //   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  0,  5,  1,  4,
    //
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    //                                               [   \   ]
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  9, 14, 10
    };

//
//  Operands
//

unsigned short Parser::operands[22][15] =
    { { 0x0d15, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0015, 0x0010, 0x0011 }
    , { 0x0111, 0x1000, 0x0b00, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0114, 0x0200, 0x0211, 0x0114 }
    , { 0x0111, 0x1001, 0x0b01, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0215, 0x0211, 0x0414 }
    , { 0x0111, 0x1001, 0x0b01, 0x0111, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0315, 0x0211, 0x0414 }
    , { 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0606, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0414, 0x0515, 0x0211, 0x0414 }
    , { 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0606, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0311, 0x0515, 0x0211, 0x0311 }
    , { 0x0411, 0x0411, 0x0411, 0x0411, 0x0411, 0x0411, 0x0715, 0x0815, 0x0411, 0x0411, 0x0411, 0x0411, 0x0615, 0x0211, 0x0411 }
    , { 0x0714, 0x0714, 0x0714, 0x070e, 0x0714, 0x0714, 0x0307, 0x0714, 0x0714, 0x0714, 0x0714, 0x0714, 0x0714, 0x0211, 0x0714 }
    , { 0x0814, 0x0814, 0x0814, 0x080e, 0x0814, 0x0814, 0x0814, 0x0307, 0x0814, 0x0814, 0x0814, 0x0814, 0x0814, 0x0211, 0x0814 }
    , { 0x0111, 0x1002, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0914, 0x0915, 0x0211, 0x0914 }
    , { 0x0511, 0x0511, 0x0904, 0x0511, 0x0511, 0x0511, 0x0511, 0x0511, 0x1215, 0x0511, 0x0511, 0x0511, 0x0511, 0x0211, 0x0105 }
    , { 0x0111, 0x1012, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0111, 0x0211, 0x0111 }
    , { 0x0111, 0x0611, 0x0912, 0x0111, 0x0e12, 0x0111, 0x0111, 0x0111, 0x1212, 0x0111, 0x0111, 0x0111, 0x0111, 0x0211, 0x0113 }
    , { 0x0111, 0x0611, 0x0912, 0x0111, 0x0e12, 0x0111, 0x0111, 0x0111, 0x1212, 0x0111, 0x0111, 0x0111, 0x0111, 0x0211, 0x0113 }
    , { 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0f15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0211, 0x0e15 }
    , { 0x0e15, 0x0015, 0x0e15, 0x0e15, 0x0f15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0e15, 0x0211, 0x0e15 }
    , { 0x0c03, 0x110f, 0x110f, 0x110e, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x1014, 0x0211, 0x110f }
    , { 0x0a15, 0x110f, 0x110f, 0x110e, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x110f, 0x0211, 0x110f }
    , { 0x0711, 0x0711, 0x0711, 0x0711, 0x0711, 0x0711, 0x0711, 0x0711, 0x0711, 0x130c, 0x0711, 0x1408, 0x0711, 0x0211, 0x1515 }
    , { 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x130f, 0x110d, 0x130f, 0x130f, 0x0211, 0x130f }
    , { 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x1415, 0x0009, 0x1415, 0x0211, 0x1415 }
    , { 0x150a, 0x000b, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x1515, 0x0211, 0x1515 }
    };

//
//  Error Strings
//

const char* Parser::errorStr[] =
    { "expected Element"
    , "unexpected character in tag"
    , "unexpected end of file found"
    , "attribute name not followed by '='"
    , "invalid attribute value"
    , "expecting end tag"
    , "empty tag"
    , "unexpected character after <!"
    };
