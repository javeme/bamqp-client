//   NOTE: this is a machine generated file--editing not recommended
//
// user.h - class definitions for ASN.1 module QvVNM
//
//   This file was generated by snacc on Wed Jul 16 17:19:49 2014
//   UBC snacc by Mike Sample

#ifndef _user_h_
#define _user_h_


#include "asn-incl.h"
#include "asn-listset.h"

#ifndef NO_NAMESPACE
namespace SNACC {
#endif
//------------------------------------------------------------------------------
// class declarations:

class AsnUserLoginReqeust;
class AsnUserLoginResponse;

//------------------------------------------------------------------------------
// externs for value defs

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// class definitions:

class  AsnUserLoginReqeust: public AsnType
{
public:
  AsnOcts       name;


  AsnOcts       password;


   AsnUserLoginReqeust() {Init();}
   void Init(void);
   virtual ~AsnUserLoginReqeust() {Clear();}
   void Clear();

	int checkConstraints(ConstraintFailList* pConstraintFails) const;

   AsnUserLoginReqeust(const AsnUserLoginReqeust& that);
public:
	 virtual const char* typeName() const	{ return "AsnUserLoginReqeust"; }
  AsnType		*Clone() const;

  AsnUserLoginReqeust		&operator = (const AsnUserLoginReqeust &that);
  AsnLen		BEncContent (AsnBuf &_b) const;
  void			BDecContent (const AsnBuf &_b, AsnTag tag, AsnLen elmtLen, AsnLen &bytesDecoded);

  AsnLen		BEnc (AsnBuf &_b) const;
  void			BDec (const AsnBuf &_b, AsnLen &bytesDecoded);
	void Print(std::ostream& os, unsigned short indent = 0) const;
  void		PrintXML (std::ostream &os, const char *lpszTitle=NULL) const;
};


class  AsnUserLoginResponse: public AsnType
{
public:
  AsnBool       success;


  AsnOcts       error;


  AsnInt       counter;


   AsnUserLoginResponse() {Init();}
   void Init(void);
   virtual ~AsnUserLoginResponse() {Clear();}
   void Clear();

	int checkConstraints(ConstraintFailList* pConstraintFails) const;

   AsnUserLoginResponse(const AsnUserLoginResponse& that);
public:
	 virtual const char* typeName() const	{ return "AsnUserLoginResponse"; }
  AsnType		*Clone() const;

  AsnUserLoginResponse		&operator = (const AsnUserLoginResponse &that);
  AsnLen		BEncContent (AsnBuf &_b) const;
  void			BDecContent (const AsnBuf &_b, AsnTag tag, AsnLen elmtLen, AsnLen &bytesDecoded);

  AsnLen		BEnc (AsnBuf &_b) const;
  void			BDec (const AsnBuf &_b, AsnLen &bytesDecoded);
	void Print(std::ostream& os, unsigned short indent = 0) const;
  void		PrintXML (std::ostream &os, const char *lpszTitle=NULL) const;
};


#ifndef NO_NAMESPACE
} // namespace close
#endif

#endif /* conditional include of user.h */
