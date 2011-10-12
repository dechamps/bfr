//
//  MinML
//
//  Derivative work of the Java version by John Wilson.
//

#ifndef _MinML_
#define _MinML_

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//

#ifndef MinMLInitialBufferSize
#define MinMLInitialBufferSize  256
#endif
#ifndef MinMLBufferIncrement
#define MinMLBufferIncrement    128
#endif

namespace MinML {

class AttributeList;

//
//  MLHandler
//

class Handler {
    public:
        virtual void StartDocument(void);
        virtual void StartElement(const char *name, const AttributeList& attrs);
        virtual void EndElement(const char *name);
        virtual void Characters(char *ch);
//      virtual void ProcessingInstruction(char* target, char* data);
        virtual void EndDocument(void);
        virtual void FatalError(const char *err, int line, int col);
    };

typedef Handler *HandlerPtr;

//
//  AttributeList
//

class AttributeList {
        friend class Parser;
        
    private:
        int     size;
        char    **attributeNames;
        char    **attributeValues;
        
        void SaveName(const char *name);
        void SaveValue(const char *value);
        void Flush(void);
        
    public:
        AttributeList( void );
        ~AttributeList( void );
        
        inline int getLength(void) const { return size; }
        
        inline const char* getName(const int i) const { return attributeNames[i]; }
        inline const char* getValue(const int i) const { return attributeValues[i]; }
        inline const char* operator [](const int i) const { return attributeValues[i]; }

        const char* getValue(const char* name) const;
        inline const char* operator [](const char* name) const { return getValue(name); }
    };

typedef AttributeList *AttributeListPtr;

//
//  Stack
//

class Stack {
    private:
        char*   buffer;
        int     size;
        char*   top;
        
    public:
        Stack(void);
        ~Stack(void);
        
        void Push(const char *name);
        const char* Pop(void);
        const char* Peek(void);
        
        inline bool Empty(void) { return (top == buffer); }
        inline void Flush(void) { top = buffer; }
    };

typedef Stack *StackPtr;
        
//
//  Buffer
//

class Buffer {
    private:
        char*               buffer;
        int                 size;
        char*               end;
        
        HandlerPtr          &handler;
        
    public:
        Buffer(HandlerPtr &hp);
        ~Buffer(void);
        
        inline void Reset(void) { end = buffer; }
        
        void Write(const char c);
        void Flush(void);
        const char* GetString(void);
    };

typedef Buffer *BufferPtr;

//
//  Parser
//

class Parser : public Handler {
    public:
        Parser(void);
        virtual ~Parser(void);
        
        virtual char* Reader(void) = 0;             // return strings, 0 iff EOF
        
        void Parse(void);                   // parse the contents
        
        void SetHandler( HandlerPtr hp );
        
    protected:
        int             lineNumber;
        int             columnNumber;
        
        const char*     currentString;
        HandlerPtr      currentHandler;
        
    private:
        enum opCodes
            { endStartName = 0
            , emitStartElement = 1
            , emitEndElement = 2
            , possiblyEmitCharacters = 3
            , emitCharacters = 4
            , emitCharactersSave = 5
            , saveAttributeName = 6
            , saveAttributeValue = 7
            , startComment = 8
            , endComment = 9
            , incLevel = 10
            , decLevel = 11
            , startCDATA = 12
            , endCDATA = 13
            , processCharRef = 14
            , writeCdata = 15
            , exitParser = 16
            , parseError = 17
            , discardAndChange = 18
            , discardSaveAndChange = 19
            , saveAndChange = 20
            , change = 21
            };
        enum states
            { inSkipping = 0
            , inSTag = 1
            , inPossiblyAttribute = 2
            , inNextAttribute = 3
            , inAttribute = 4
            , inAttribute1 = 5
            , inAttributeValue = 6
            , inAttributeQuoteValue = 7
            , inAttributeQuotesValue = 8
            , inETag = 9
            , inETag1 = 10
            , inMTTag = 11
            , inTag = 12
            , inTag1 = 13
            , inPI = 14
            , inPI1 = 15
            , inPossiblySkipping = 16
            , inCharData = 17
            , inCDATA = 18
            , inCDATA1 = 19
            , inComment = 20
            , inDTD = 21
            };
            
        char*                   elementName;
        AttributeList           attrs;
        Buffer                  buffer;
        Stack                   tags;
        
        static char             charClasses[];
        static unsigned short   operands[22][15];
        static const char*      errorStr[];
        
        char    Read(void);
        int     Digit(char ch, int radix);
    };

typedef Parser *ParserPtr;

}

#endif
