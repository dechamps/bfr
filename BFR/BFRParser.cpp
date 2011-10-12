//
//  BFRParser
//

#include <stdio.h>
#include <string.h>

#include "BFRParser.h"
#include "BFRRegistration.h"

//
//  Null Factory
//

BFRFactory gNullFactory(0);

//
//  BFRParser::BFRParser
//

BFRParser::BFRParser( const char *rootName, BFRFactoryPtr rootFactory )
    : parserRootName(rootName), parserRoot(rootFactory)
    , parserFile(0), parserStack(0)
{
}

//
//  BFRParser::~BFRParser
//

BFRParser::~BFRParser( void )
{
}

//
//  BFRParser::Parse
//

void BFRParser::Parse(const char *fileName)
{
    // open the input file
    parserFile = fopen( fileName, "rb" );
    if (!parserFile) {
        fprintf( stderr, "Unable to open configuration file: %s\n", fileName );
        return;
    }
    
    // parse the contents
    this->MinML::Parser::Parse();
    
    // make sure everything is lined up correctly
    gBFRRegistration.ValidateRegistrations();
}

//
//  BFRParser::Reader
//

char* BFRParser::Reader(void)
{
    int     len
    ;
    
    // no file returns no data
    if (!parserFile)
        return 0;
        
    // read some
    len = fread(parserLine, 1, kParserLineSize-1, parserFile);
    
    // check for end-of-file
    if (feof(parserFile)) {
        fclose(parserFile);
        parserFile = 0;
    }
    
    // return the data found
    parserLine[len] = 0;
    return parserLine;
}

//
//  BFRParser::StartElement
//

void BFRParser::StartElement(const char *name, const MinML::AttributeList& attrs)
{
    BFRParser::StackElem    *newElem
    ;

    // create a new stack element
    newElem = new BFRParser::StackElem();
    newElem->stackFactory = &gNullFactory;
    newElem->stackElement = 0;
    newElem->stackID = -1;
    newElem->stackPrev = parserStack;

    // if there's already a factory on the stack, look through its list of acceptable children
    if (parserStack) {
        BFRFactoryChildPtr  childList = parserStack->stackFactory->factoryChildren
        ;

        // look for a match
        if (childList)
            for (int i = 0; childList[i].childFactory; i++)
                if (strcmp(name,childList[i].childName) == 0) {
                    newElem->stackFactory = childList[i].childFactory;
                    newElem->stackID = i;
                    break;
                }
    } else
    if (strcmp(name,parserRootName) == 0) {
        newElem->stackFactory = parserRoot;
        newElem->stackID = 0;
    }
    
    // ask the factory for an element
    newElem->stackElement = newElem->stackFactory->StartElement( name, attrs );
    
    // finish pushing the new element on the stack
    parserStack = newElem;
}

//
//  BFRParser::EndElement
//

void BFRParser::EndElement(const char *name)
{
    BFRParser::StackElem    *stackTop = parserStack
    ;

    // check for a factory
    if (stackTop) {
        // end the element
        stackTop->stackFactory->EndElement( parserStack->stackElement );

        // pop the stack
        BFRParser::StackElem *oldTop = parserStack;
        parserStack = stackTop = oldTop->stackPrev;

        // if there is someone to tell, tell them about the element that has been created.  Otherwise this 
        // must be the root document, so store it for the application.
        if (stackTop) {
            // only tell about valid elements
            if (oldTop->stackID >= 0)
                stackTop->stackFactory->ChildElement( stackTop->stackElement, oldTop->stackID, oldTop->stackElement );
        } else
            parserElem = oldTop->stackElement;

        // delete the old top
        delete oldTop;
    }
}

//
//  SubstitutionEnvironment
//

char **gARGV, **gENVP;

void SubstitutionEnvironment(char* argv[], char* envp[])
{
    gARGV = argv;
    gENVP = envp;
}

//
//  SubstituteArgs
//

const char *SubstituteArgs(const char *s)
{
    int         i, j, slen
    ;
    const char  *envs
    ;
    
    // don't process null or empty strings
    if ((!s) || (!*s))
        return s;
        
    // if the first character is not '$', return as is
    if (s[0] != '$')
        return s;
    
    // bump to the next character
    s += 1;
    
    // if the first character is '$', do a recursive substitution
    // which may or may not be very useful.
    if (s[0] != '$')
        s = SubstituteArgs(s);
    
    // look for a digit
    if (isdigit(*s)) {
        i = 0;
        while (isdigit(*s))
            i = (i * 10) + (*s++ - '0');
            
        for (j = 0; (j != i) && gARGV[j]; j++)
            ;
        if (gARGV[j])
            return gARGV[j];
            
        throw_1(1010);     // not enough arguments
    }
    
    // look for a matching env variable
    slen = strlen(s);
    for (j = 0; gENVP[j]; j++) {
        envs = gENVP[j];
        if ((strncmp(envs, s, slen) == 0) && (envs[slen] == '='))
            return envs + slen + 1;
    }
    
    throw_2(1011, s);     // unmatched environment variable
}
