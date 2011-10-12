//
//  BFRParser
//

#ifndef _BFRParser_
#define _BFRParser_

#include <stdio.h>

#include "MinML.h"

// 

typedef void *voidPtr;

//
//  BFRFactory
//

class BFRFactory;
typedef BFRFactory *BFRFactoryPtr;

struct BFRFactoryChild {
    const char          *childName;
    BFRFactoryPtr       childFactory;
    };

typedef BFRFactoryChild *BFRFactoryChildPtr;

class BFRFactory {
    public:
        BFRFactoryChildPtr factoryChildren;
        
        BFRFactory( BFRFactoryChildPtr fcp = 0 )
            : factoryChildren(fcp)
        { }

        virtual voidPtr StartElement( const char *name, const MinML::AttributeList& attrs )
        {
            return 0;
        }

        virtual void ChildElement( voidPtr ep, int id, voidPtr cp )
        { }

        virtual void EndElement( voidPtr ep )
        { }
    };

typedef BFRFactory *BFRFactoryPtr;

extern BFRFactory gNullFactory;

//
//  BFRParser
//

const int kParserLineSize = 1024;

class BFRParser : public MinML::Parser {

    protected:
        struct StackElem {
            StackElem       *stackPrev;             // previous stack element
            BFRFactoryPtr   stackFactory;           // current factory
            voidPtr         stackElement;           // current element
            int             stackID;                // list match number
            };

        const char          *parserRootName;        // root name
        BFRFactoryPtr       parserRoot;             // root factory
        FILE                *parserFile;            // incoming data
        char                parserLine[kParserLineSize];    // line buffer
        StackElem           *parserStack;           // pointer to stack

        char* Reader(void);                         // feed strings to parser
        
        void StartElement(const char *name, const MinML::AttributeList& attrs);
        void EndElement(const char *name);
        
    public:
        voidPtr             parserElem;             // a pointer to the element that was built

        BFRParser( const char *rootName, BFRFactoryPtr rootFactory );
        ~BFRParser( void );

        void                Parse(const char *fileName);
    };

typedef BFRParser *BFRParserPtr;

//

void SubstitutionEnvironment(char* argv[], char* envp[]);
const char *SubstituteArgs(const char *s);

#endif
