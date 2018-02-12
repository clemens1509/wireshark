/* Do not modify this file. Changes will be overwritten.                      */
/* Generated automatically by the ASN.1 to Wireshark dissector compiler       */
/* packet-spnego.c                                                            */
/* asn2wrs.py -b -p spnego -c ./spnego.cnf -s ./packet-spnego-template -D . -O ../.. spnego.asn */

/* Input file: packet-spnego-template.c */

#line 1 "./asn1/spnego/packet-spnego-template.c"
/* packet-spnego-template.c
 * Routines for the simple and protected GSS-API negotiation mechanism
 * as described in RFC 2478.
 * Copyright 2002, Tim Potter <tpot@samba.org>
 * Copyright 2002, Richard Sharpe <rsharpe@ns.aus.com>
 * Copyright 2003, Richard Sharpe <rsharpe@richardsharpe.com>
 * Copyright 2005, Ronnie Sahlberg (krb decryption)
 * Copyright 2005, Anders Broman (converted to asn2wrs generated dissector)
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
/* The heimdal code for decryption of GSSAPI wrappers using heimdal comes from
   Heimdal 1.6 and has been modified for wireshark's requirements.
*/

#include "config.h"

#include <epan/packet.h>
#include <epan/expert.h>
#include <epan/asn1.h>
#include <epan/conversation.h>
#include <epan/proto_data.h>
#include <wsutil/wsgcrypt.h>
#include "packet-dcerpc.h"
#include "packet-gssapi.h"
#include "packet-kerberos.h"
#include "packet-ber.h"

#define PNAME  "Simple Protected Negotiation"
#define PSNAME "SPNEGO"
#define PFNAME "spnego"

void proto_register_spnego(void);
void proto_reg_handoff_spnego(void);

/* Initialize the protocol and registered fields */
static int proto_spnego = -1;
static int proto_spnego_krb5 = -1;


static int hf_spnego_wraptoken = -1;
static int hf_spnego_krb5_oid;
static int hf_spnego_krb5 = -1;
static int hf_spnego_krb5_tok_id = -1;
static int hf_spnego_krb5_sgn_alg = -1;
static int hf_spnego_krb5_seal_alg = -1;
static int hf_spnego_krb5_snd_seq = -1;
static int hf_spnego_krb5_sgn_cksum = -1;
static int hf_spnego_krb5_confounder = -1;
static int hf_spnego_krb5_filler = -1;
static int hf_spnego_krb5_cfx_flags = -1;
static int hf_spnego_krb5_cfx_flags_01 = -1;
static int hf_spnego_krb5_cfx_flags_02 = -1;
static int hf_spnego_krb5_cfx_flags_04 = -1;
static int hf_spnego_krb5_cfx_ec = -1;
static int hf_spnego_krb5_cfx_rrc = -1;
static int hf_spnego_krb5_cfx_seq = -1;


/*--- Included file: packet-spnego-hf.c ---*/
#line 1 "./asn1/spnego/packet-spnego-hf.c"
static int hf_spnego_negTokenInit = -1;           /* T_negTokenInit */
static int hf_spnego_negTokenTarg = -1;           /* NegTokenTarg */
static int hf_spnego_MechTypeList_item = -1;      /* MechType */
static int hf_spnego_mechTypes = -1;              /* MechTypeList */
static int hf_spnego_reqFlags = -1;               /* ContextFlags */
static int hf_spnego_mechToken = -1;              /* T_mechToken */
static int hf_spnego_mechListMIC = -1;            /* OCTET_STRING */
static int hf_spnego_hintName = -1;               /* GeneralString */
static int hf_spnego_hintAddress = -1;            /* OCTET_STRING */
static int hf_spnego_mechToken_01 = -1;           /* OCTET_STRING */
static int hf_spnego_negHints = -1;               /* NegHints */
static int hf_spnego_negResult = -1;              /* T_negResult */
static int hf_spnego_supportedMech = -1;          /* T_supportedMech */
static int hf_spnego_responseToken = -1;          /* T_responseToken */
static int hf_spnego_thisMech = -1;               /* MechType */
static int hf_spnego_innerContextToken = -1;      /* InnerContextToken */
/* named bits */
static int hf_spnego_ContextFlags_delegFlag = -1;
static int hf_spnego_ContextFlags_mutualFlag = -1;
static int hf_spnego_ContextFlags_replayFlag = -1;
static int hf_spnego_ContextFlags_sequenceFlag = -1;
static int hf_spnego_ContextFlags_anonFlag = -1;
static int hf_spnego_ContextFlags_confFlag = -1;
static int hf_spnego_ContextFlags_integFlag = -1;

/*--- End of included file: packet-spnego-hf.c ---*/
#line 76 "./asn1/spnego/packet-spnego-template.c"

/* Global variables */
static const char *MechType_oid;
gssapi_oid_value *next_level_value;
gboolean saw_mechanism = FALSE;


/* Initialize the subtree pointers */
static gint ett_spnego = -1;
static gint ett_spnego_wraptoken = -1;
static gint ett_spnego_krb5 = -1;
static gint ett_spnego_krb5_cfx_flags = -1;


/*--- Included file: packet-spnego-ett.c ---*/
#line 1 "./asn1/spnego/packet-spnego-ett.c"
static gint ett_spnego_NegotiationToken = -1;
static gint ett_spnego_MechTypeList = -1;
static gint ett_spnego_NegTokenInit = -1;
static gint ett_spnego_NegHints = -1;
static gint ett_spnego_NegTokenInit2 = -1;
static gint ett_spnego_ContextFlags = -1;
static gint ett_spnego_NegTokenTarg = -1;
static gint ett_spnego_InitialContextToken_U = -1;

/*--- End of included file: packet-spnego-ett.c ---*/
#line 90 "./asn1/spnego/packet-spnego-template.c"

static expert_field ei_spnego_decrypted_keytype = EI_INIT;
static expert_field ei_spnego_unknown_header = EI_INIT;

static dissector_handle_t spnego_handle;
static dissector_handle_t spnego_krb5_handle;
static dissector_handle_t spnego_krb5_wrap_handle;

/*
 * Unfortunately, we have to have forward declarations of thess,
 * as the code generated by asn2wrs includes a call before the
 * definition.
 */
static int dissect_spnego_NegTokenInit(gboolean implicit_tag, tvbuff_t *tvb,
                                       int offset, asn1_ctx_t *actx _U_,
                                       proto_tree *tree, int hf_index);
static int dissect_spnego_NegTokenInit2(gboolean implicit_tag, tvbuff_t *tvb,
                                        int offset, asn1_ctx_t *actx _U_,
                                        proto_tree *tree, int hf_index);


/*--- Included file: packet-spnego-fn.c ---*/
#line 1 "./asn1/spnego/packet-spnego-fn.c"


static int
dissect_spnego_MechType(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 33 "./asn1/spnego/spnego.cnf"

  gssapi_oid_value *value;

  offset = dissect_ber_object_identifier_str(implicit_tag, actx, tree, tvb, offset, hf_index, &MechType_oid);


  value = gssapi_lookup_oid_str(MechType_oid);

  /*
   * Tell our caller the first mechanism we see, so that if
   * this is a negTokenInit with a mechToken, it can interpret
   * the mechToken according to the first mechType.  (There
   * might not have been any indication of the mechType
   * in prior frames, so we can't necessarily use the
   * mechanism from the conversation; i.e., a negTokenInit
   * can contain the initial security token for the desired
   * mechanism of the initiator - that's the first mechanism
   * in the list.)
   */
  if (!saw_mechanism) {
    if (value)
      next_level_value = value;
    saw_mechanism = TRUE;
  }



  return offset;
}


static const ber_sequence_t MechTypeList_sequence_of[1] = {
  { &hf_spnego_MechTypeList_item, BER_CLASS_UNI, BER_UNI_TAG_OID, BER_FLAGS_NOOWNTAG, dissect_spnego_MechType },
};

static int
dissect_spnego_MechTypeList(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 101 "./asn1/spnego/spnego.cnf"

  conversation_t *conversation;

  saw_mechanism = FALSE;

  offset = dissect_ber_sequence_of(implicit_tag, actx, tree, tvb, offset,
                                      MechTypeList_sequence_of, hf_index, ett_spnego_MechTypeList);


  /*
   * If we saw a mechType we need to store it in case the negTokenTarg
   * does not provide a supportedMech.
   */
  if(saw_mechanism){
    conversation = find_or_create_conversation(actx->pinfo);
    conversation_add_proto_data(conversation, proto_spnego, next_level_value);
  }



  return offset;
}


static const asn_namedbit ContextFlags_bits[] = {
  {  0, &hf_spnego_ContextFlags_delegFlag, -1, -1, "delegFlag", NULL },
  {  1, &hf_spnego_ContextFlags_mutualFlag, -1, -1, "mutualFlag", NULL },
  {  2, &hf_spnego_ContextFlags_replayFlag, -1, -1, "replayFlag", NULL },
  {  3, &hf_spnego_ContextFlags_sequenceFlag, -1, -1, "sequenceFlag", NULL },
  {  4, &hf_spnego_ContextFlags_anonFlag, -1, -1, "anonFlag", NULL },
  {  5, &hf_spnego_ContextFlags_confFlag, -1, -1, "confFlag", NULL },
  {  6, &hf_spnego_ContextFlags_integFlag, -1, -1, "integFlag", NULL },
  { 0, NULL, 0, 0, NULL, NULL }
};

static int
dissect_spnego_ContextFlags(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_bitstring(implicit_tag, actx, tree, tvb, offset,
                                    ContextFlags_bits, hf_index, ett_spnego_ContextFlags,
                                    NULL);

  return offset;
}



static int
dissect_spnego_T_mechToken(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 122 "./asn1/spnego/spnego.cnf"

  tvbuff_t *mechToken_tvb = NULL;

  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &mechToken_tvb);


  /*
   * Now, we should be able to dispatch, if we've gotten a tvbuff for
   * the token and we have information on how to dissect its contents.
   */
  if (mechToken_tvb && next_level_value)
     call_dissector(next_level_value->handle, mechToken_tvb, actx->pinfo, tree);



  return offset;
}



static int
dissect_spnego_OCTET_STRING(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       NULL);

  return offset;
}


static const ber_sequence_t NegTokenInit_sequence[] = {
  { &hf_spnego_mechTypes    , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL, dissect_spnego_MechTypeList },
  { &hf_spnego_reqFlags     , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL, dissect_spnego_ContextFlags },
  { &hf_spnego_mechToken    , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL, dissect_spnego_T_mechToken },
  { &hf_spnego_mechListMIC  , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL, dissect_spnego_OCTET_STRING },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_NegTokenInit(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NegTokenInit_sequence, hf_index, ett_spnego_NegTokenInit);

  return offset;
}



static int
dissect_spnego_T_negTokenInit(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 12 "./asn1/spnego/spnego.cnf"
  gboolean is_response = actx->pinfo->ptype == PT_TCP &&
                         actx->pinfo->srcport < 1024;

  /*
   * We decode as negTokenInit2 or negTokenInit depending on whether or not
   * we are in a response or a request. That is essentially what MS-SPNG
   * says.
   */
  if (is_response) {
    return dissect_spnego_NegTokenInit2(implicit_tag, tvb, offset,
                                        actx, tree, hf_index);
  } else {
    return dissect_spnego_NegTokenInit(implicit_tag, tvb, offset,
                                       actx, tree, hf_index);
  }



  return offset;
}


static const value_string spnego_T_negResult_vals[] = {
  {   0, "accept-completed" },
  {   1, "accept-incomplete" },
  {   2, "reject" },
  { 0, NULL }
};


static int
dissect_spnego_T_negResult(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_integer(implicit_tag, actx, tree, tvb, offset, hf_index,
                                  NULL);

  return offset;
}



static int
dissect_spnego_T_supportedMech(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 135 "./asn1/spnego/spnego.cnf"

  conversation_t *conversation;

  saw_mechanism = FALSE;

  offset = dissect_spnego_MechType(implicit_tag, tvb, offset, actx, tree, hf_index);


  /*
   * If we saw an explicit mechType we store this in the conversation so that
   * it will override any mechType we might have picked up from the
   * negTokenInit.
   */
  if(saw_mechanism){
    conversation = find_or_create_conversation(actx->pinfo);
    conversation_add_proto_data(conversation, proto_spnego, next_level_value);
  }




  return offset;
}



static int
dissect_spnego_T_responseToken(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 158 "./asn1/spnego/spnego.cnf"

  tvbuff_t *responseToken_tvb;


  offset = dissect_ber_octet_string(implicit_tag, actx, tree, tvb, offset, hf_index,
                                       &responseToken_tvb);



  /*
   * Now, we should be able to dispatch, if we've gotten a tvbuff for
   * the token and we have information on how to dissect its contents.
   * However, we should make sure that there is something in the
   * response token ...
   */
  if (responseToken_tvb && (tvb_reported_length(responseToken_tvb) > 0) ){
    gssapi_oid_value *value=next_level_value;

    if(value){
      call_dissector(value->handle, responseToken_tvb, actx->pinfo, tree);
    }
  }



  return offset;
}


static const ber_sequence_t NegTokenTarg_sequence[] = {
  { &hf_spnego_negResult    , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL, dissect_spnego_T_negResult },
  { &hf_spnego_supportedMech, BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL, dissect_spnego_T_supportedMech },
  { &hf_spnego_responseToken, BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL, dissect_spnego_T_responseToken },
  { &hf_spnego_mechListMIC  , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL, dissect_spnego_OCTET_STRING },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_NegTokenTarg(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NegTokenTarg_sequence, hf_index, ett_spnego_NegTokenTarg);

  return offset;
}


static const ber_choice_t NegotiationToken_choice[] = {
  {   0, &hf_spnego_negTokenInit , BER_CLASS_CON, 0, 0, dissect_spnego_T_negTokenInit },
  {   1, &hf_spnego_negTokenTarg , BER_CLASS_CON, 1, 0, dissect_spnego_NegTokenTarg },
  { 0, NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_NegotiationToken(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_choice(actx, tree, tvb, offset,
                                 NegotiationToken_choice, hf_index, ett_spnego_NegotiationToken,
                                 NULL);

  return offset;
}



static int
dissect_spnego_GeneralString(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_restricted_string(implicit_tag, BER_UNI_TAG_GeneralString,
                                            actx, tree, tvb, offset, hf_index,
                                            NULL);

  return offset;
}


static const ber_sequence_t NegHints_sequence[] = {
  { &hf_spnego_hintName     , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL, dissect_spnego_GeneralString },
  { &hf_spnego_hintAddress  , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL, dissect_spnego_OCTET_STRING },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_NegHints(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NegHints_sequence, hf_index, ett_spnego_NegHints);

  return offset;
}


static const ber_sequence_t NegTokenInit2_sequence[] = {
  { &hf_spnego_mechTypes    , BER_CLASS_CON, 0, BER_FLAGS_OPTIONAL, dissect_spnego_MechTypeList },
  { &hf_spnego_reqFlags     , BER_CLASS_CON, 1, BER_FLAGS_OPTIONAL, dissect_spnego_ContextFlags },
  { &hf_spnego_mechToken_01 , BER_CLASS_CON, 2, BER_FLAGS_OPTIONAL, dissect_spnego_OCTET_STRING },
  { &hf_spnego_negHints     , BER_CLASS_CON, 3, BER_FLAGS_OPTIONAL, dissect_spnego_NegHints },
  { &hf_spnego_mechListMIC  , BER_CLASS_CON, 4, BER_FLAGS_OPTIONAL, dissect_spnego_OCTET_STRING },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_NegTokenInit2(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   NegTokenInit2_sequence, hf_index, ett_spnego_NegTokenInit2);

  return offset;
}



static int
dissect_spnego_InnerContextToken(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
#line 58 "./asn1/spnego/spnego.cnf"

  gssapi_oid_value *next_level_value_lcl;
  proto_item *item;
  proto_tree *subtree;
  tvbuff_t *token_tvb;
  int len;

  /*
   * XXX - what should we do if this OID doesn't match the value
   * attached to the frame or conversation?  (That would be
   * bogus, but that's not impossible - some broken implementation
   * might negotiate some security mechanism but put the OID
   * for some other security mechanism in GSS_Wrap tokens.)
   * Does it matter?
   */
  next_level_value_lcl = gssapi_lookup_oid_str(MechType_oid);

  /*
   * Now dissect the GSS_Wrap token; it's assumed to be in the
   * rest of the tvbuff.
   */
  item = proto_tree_add_item(tree, hf_spnego_wraptoken, tvb, offset, -1, ENC_NA);

  subtree = proto_item_add_subtree(item, ett_spnego_wraptoken);

  /*
   * Now, we should be able to dispatch after creating a new TVB.
   * The subdissector must return the length of the part of the
   * token it dissected, so we can return the length of the part
   * we (and it) dissected.
   */
  token_tvb = tvb_new_subset_remaining(tvb, offset);
  if (next_level_value_lcl && next_level_value_lcl->wrap_handle) {
    len = call_dissector(next_level_value_lcl->wrap_handle, token_tvb, actx->pinfo,
                         subtree);
    if (len == 0)
      offset = tvb_reported_length(tvb);
    else
      offset = offset + len;
  } else
    offset = tvb_reported_length(tvb);



  return offset;
}


static const ber_sequence_t InitialContextToken_U_sequence[] = {
  { &hf_spnego_thisMech     , BER_CLASS_UNI, BER_UNI_TAG_OID, BER_FLAGS_NOOWNTAG, dissect_spnego_MechType },
  { &hf_spnego_innerContextToken, BER_CLASS_ANY, 0, BER_FLAGS_NOOWNTAG, dissect_spnego_InnerContextToken },
  { NULL, 0, 0, 0, NULL }
};

static int
dissect_spnego_InitialContextToken_U(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_sequence(implicit_tag, actx, tree, tvb, offset,
                                   InitialContextToken_U_sequence, hf_index, ett_spnego_InitialContextToken_U);

  return offset;
}



static int
dissect_spnego_InitialContextToken(gboolean implicit_tag _U_, tvbuff_t *tvb _U_, int offset _U_, asn1_ctx_t *actx _U_, proto_tree *tree _U_, int hf_index _U_) {
  offset = dissect_ber_tagged_type(implicit_tag, actx, tree, tvb, offset,
                                      hf_index, BER_CLASS_APP, 0, TRUE, dissect_spnego_InitialContextToken_U);

  return offset;
}


/*--- End of included file: packet-spnego-fn.c ---*/
#line 111 "./asn1/spnego/packet-spnego-template.c"
/*
 * This is the SPNEGO KRB5 dissector. It is not true KRB5, but some ASN.1
 * wrapped blob with an OID, USHORT token ID, and a Ticket, that is also
 * ASN.1 wrapped by the looks of it. It conforms to RFC1964.
 */

#define KRB_TOKEN_AP_REQ              0x0001
#define KRB_TOKEN_AP_REP              0x0002
#define KRB_TOKEN_AP_ERR              0x0003
#define KRB_TOKEN_GETMIC              0x0101
#define KRB_TOKEN_WRAP                0x0102
#define KRB_TOKEN_DELETE_SEC_CONTEXT  0x0201
#define KRB_TOKEN_CFX_GETMIC          0x0404
#define KRB_TOKEN_CFX_WRAP            0x0405

static const value_string spnego_krb5_tok_id_vals[] = {
  { KRB_TOKEN_AP_REQ,             "KRB5_AP_REQ"},
  { KRB_TOKEN_AP_REP,             "KRB5_AP_REP"},
  { KRB_TOKEN_AP_ERR,             "KRB5_ERROR"},
  { KRB_TOKEN_GETMIC,             "KRB5_GSS_GetMIC" },
  { KRB_TOKEN_WRAP,               "KRB5_GSS_Wrap" },
  { KRB_TOKEN_DELETE_SEC_CONTEXT, "KRB5_GSS_Delete_sec_context" },
  { KRB_TOKEN_CFX_GETMIC,         "KRB_TOKEN_CFX_GetMic" },
  { KRB_TOKEN_CFX_WRAP,            "KRB_TOKEN_CFX_WRAP" },
  { 0, NULL}
};

#define KRB_SGN_ALG_DES_MAC_MD5  0x0000
#define KRB_SGN_ALG_MD2_5  0x0001
#define KRB_SGN_ALG_DES_MAC  0x0002
#define KRB_SGN_ALG_HMAC  0x0011

static const value_string spnego_krb5_sgn_alg_vals[] = {
  { KRB_SGN_ALG_DES_MAC_MD5, "DES MAC MD5"},
  { KRB_SGN_ALG_MD2_5,       "MD2.5"},
  { KRB_SGN_ALG_DES_MAC,     "DES MAC"},
  { KRB_SGN_ALG_HMAC,        "HMAC"},
  { 0, NULL}
};

#define KRB_SEAL_ALG_DES_CBC  0x0000
#define KRB_SEAL_ALG_RC4  0x0010
#define KRB_SEAL_ALG_NONE  0xffff

static const value_string spnego_krb5_seal_alg_vals[] = {
  { KRB_SEAL_ALG_DES_CBC, "DES CBC"},
  { KRB_SEAL_ALG_RC4,     "RC4"},
  { KRB_SEAL_ALG_NONE,    "None"},
  { 0, NULL}
};

/*
 * XXX - is this for SPNEGO or just GSS-API?
 * RFC 1964 is "The Kerberos Version 5 GSS-API Mechanism"; presumably one
 * can directly designate Kerberos V5 as a mechanism in GSS-API, rather
 * than designating SPNEGO as the mechanism, offering Kerberos V5, and
 * getting it accepted.
 */
static int
dissect_spnego_krb5_getmic_base(tvbuff_t *tvb, int offset, packet_info *pinfo, proto_tree *tree);
static int
dissect_spnego_krb5_wrap_base(tvbuff_t *tvb, int offset, packet_info *pinfo, proto_tree *tree, guint16 token_id, gssapi_encrypt_info_t* gssapi_encrypt);
static int
dissect_spnego_krb5_cfx_getmic_base(tvbuff_t *tvb, int offset, packet_info *pinfo, proto_tree *tree);
static int
dissect_spnego_krb5_cfx_wrap_base(tvbuff_t *tvb, int offset, packet_info *pinfo, proto_tree *tree, guint16 token_id, gssapi_encrypt_info_t* gssapi_encrypt);

static int
dissect_spnego_krb5(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data)
{
  proto_item *item;
  proto_tree *subtree;
  int offset = 0;
  guint16 token_id;
  const char *oid;
  tvbuff_t *krb5_tvb;
  gint8 ber_class;
  gboolean pc, ind = 0;
  gint32 tag;
  guint32 len;
  gssapi_encrypt_info_t* encrypt_info = (gssapi_encrypt_info_t*)data;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, TRUE, pinfo);

  item = proto_tree_add_item(tree, hf_spnego_krb5, tvb, offset, -1, ENC_NA);

  subtree = proto_item_add_subtree(item, ett_spnego_krb5);

  /*
   * The KRB5 blob conforms to RFC1964:
   * [APPLICATION 0] {
   *   OID,
   *   USHORT (0x0001 == AP-REQ, 0x0002 == AP-REP, 0x0003 == ERROR),
   *   OCTET STRING }
   *
   * However, for some protocols, the KRB5 blob starts at the SHORT
   * and has no DER encoded header etc.
   *
   * It appears that for some other protocols the KRB5 blob is just
   * a Kerberos message, with no [APPLICATION 0] header, no OID,
   * and no USHORT.
   *
   * So:
   *
   *  If we see an [APPLICATION 0] HEADER, we show the OID and
   *  the USHORT, and then dissect the rest as a Kerberos message.
   *
   *  If we see an [APPLICATION 14] or [APPLICATION 15] header,
   *  we assume it's an AP-REQ or AP-REP message, and dissect
   *  it all as a Kerberos message.
   *
   *  Otherwise, we show the USHORT, and then dissect the rest
   *  as a Kerberos message.
   */

  /*
   * Get the first header ...
   */
  get_ber_identifier(tvb, offset, &ber_class, &pc, &tag);
  if (ber_class == BER_CLASS_APP && pc) {
    /*
     * [APPLICATION <tag>]
    */
    offset = dissect_ber_identifier(pinfo, subtree, tvb, offset, &ber_class, &pc, &tag);
    offset = dissect_ber_length(pinfo, subtree, tvb, offset, &len, &ind);

    switch (tag) {

      case 0:
        /*
         * [APPLICATION 0]
         */

        /* Next, the OID */
        offset=dissect_ber_object_identifier_str(FALSE, &asn1_ctx, subtree, tvb, offset, hf_spnego_krb5_oid, &oid);

        token_id = tvb_get_letohs(tvb, offset);
        proto_tree_add_uint(subtree, hf_spnego_krb5_tok_id, tvb, offset, 2, token_id);

        offset += 2;

        break;

      case 14: /* [APPLICATION 14] */
      case 15: /* [APPLICATION 15] */
        /*
         * No token ID - just dissect as a Kerberos message and
         * return.
         */
        dissect_kerberos_main(tvb, pinfo, subtree, FALSE, NULL);
        return tvb_captured_length(tvb);

      default:
        proto_tree_add_expert_format(subtree, pinfo, &ei_spnego_unknown_header, tvb, offset, 0,
          "Unknown header (class=%d, pc=%d, tag=%d)", ber_class, pc, tag);
        goto done;
      }
  } else {
      /* Next, the token ID ... */

    token_id = tvb_get_letohs(tvb, offset);
    proto_tree_add_uint(subtree, hf_spnego_krb5_tok_id, tvb, offset, 2, token_id);

    offset += 2;
  }

  switch (token_id) {

    case KRB_TOKEN_AP_REQ:
    case KRB_TOKEN_AP_REP:
    case KRB_TOKEN_AP_ERR:
      krb5_tvb = tvb_new_subset_remaining(tvb, offset);
      offset = dissect_kerberos_main(krb5_tvb, pinfo, subtree, FALSE, NULL);
      break;

    case KRB_TOKEN_GETMIC:
      offset = dissect_spnego_krb5_getmic_base(tvb, offset, pinfo, subtree);
      break;

    case KRB_TOKEN_WRAP:
      offset = dissect_spnego_krb5_wrap_base(tvb, offset, pinfo, subtree, token_id, encrypt_info);
      break;

    case KRB_TOKEN_DELETE_SEC_CONTEXT:

      break;

    case KRB_TOKEN_CFX_GETMIC:
      offset = dissect_spnego_krb5_cfx_getmic_base(tvb, offset, pinfo, subtree);
      break;

    case KRB_TOKEN_CFX_WRAP:
      offset = dissect_spnego_krb5_cfx_wrap_base(tvb, offset, pinfo, subtree, token_id, encrypt_info);
      break;

    default:

      break;
  }

  done:
    proto_item_set_len(item, offset);
    return tvb_captured_length(tvb);
}

#ifdef HAVE_KERBEROS
#ifndef KEYTYPE_ARCFOUR_56
# define KEYTYPE_ARCFOUR_56 24
#endif
#ifndef KEYTYPE_ARCFOUR_HMAC
# define KEYTYPE_ARCFOUR_HMAC 23
#endif
/* XXX - We should probably do a configure-time check for this instead */
#ifndef KRB5_KU_USAGE_SEAL
# define KRB5_KU_USAGE_SEAL 22
#endif

static int
arcfour_mic_key(const guint8 *key_data, size_t key_size, int key_type,
                const guint8 *cksum_data, size_t cksum_size,
                guint8 *key6_data)
{
  guint8 k5_data[HASH_MD5_LENGTH];
  guint8 T[4] = { 0 };

  if (key_type == KEYTYPE_ARCFOUR_56) {
    guint8 L40[14] = "fortybits";
    memcpy(L40 + 10, T, sizeof(T));
    if (ws_hmac_buffer(GCRY_MD_MD5, k5_data, L40, 14, key_data, key_size)) {
      return 0;
    }
    memset(&k5_data[7], 0xAB, 9);
  } else {
    if (ws_hmac_buffer(GCRY_MD_MD5, k5_data, T, 4, key_data, key_size)) {
      return 0;
    }
  }

  if (ws_hmac_buffer(GCRY_MD_MD5, key6_data, cksum_data, cksum_size, k5_data, HASH_MD5_LENGTH)) {
    return 0;
  }
  return 0;
}

static int
usage2arcfour(int usage)
{
  switch (usage) {
    case 3: /*KRB5_KU_AS_REP_ENC_PART 3 */
    case 9: /*KRB5_KU_TGS_REP_ENC_PART_SUB_KEY 9 */
      return 8;
    case 22: /*KRB5_KU_USAGE_SEAL 22 */
      return 13;
    case 23: /*KRB5_KU_USAGE_SIGN 23 */
        return 15;
    case 24: /*KRB5_KU_USAGE_SEQ 24 */
      return 0;
    default :
    return 0;
  }
}

static int
arcfour_mic_cksum(guint8 *key_data, int key_length,
                  unsigned int usage,
                  guint8 sgn_cksum[8],
                  const guint8 *v1, size_t l1,
                  const guint8 *v2, size_t l2,
                  const guint8 *v3, size_t l3)
{
  static const guint8 signature[] = "signaturekey";
  guint8 ksign_c[HASH_MD5_LENGTH];
  guint8 t[4];
  guint8 digest[HASH_MD5_LENGTH];
  int rc4_usage;
  guint8 cksum[HASH_MD5_LENGTH];
  gcry_md_hd_t md5_handle;

  rc4_usage=usage2arcfour(usage);
  if (ws_hmac_buffer(GCRY_MD_MD5, ksign_c, signature, sizeof(signature), key_data, key_length)) {
    return 0;
  }

  if (gcry_md_open(&md5_handle, GCRY_MD_MD5, 0)) {
    return 0;
  }
  t[0] = (rc4_usage >>  0) & 0xFF;
  t[1] = (rc4_usage >>  8) & 0xFF;
  t[2] = (rc4_usage >> 16) & 0xFF;
  t[3] = (rc4_usage >> 24) & 0xFF;
  gcry_md_write(md5_handle, t, 4);
  gcry_md_write(md5_handle, v1, l1);
  gcry_md_write(md5_handle, v2, l2);
  gcry_md_write(md5_handle, v3, l3);
  memcpy(digest, gcry_md_read(md5_handle, 0), HASH_MD5_LENGTH);
  gcry_md_close(md5_handle);

  if (ws_hmac_buffer(GCRY_MD_MD5, cksum, digest, HASH_MD5_LENGTH, ksign_c, HASH_MD5_LENGTH)) {
    return 0;
  }

  memcpy(sgn_cksum, cksum, 8);

  return 0;
}

/*
 * Verify padding of a gss wrapped message and return its length.
 */
static int
gssapi_verify_pad(guint8 *wrapped_data, int wrapped_length,
                  int datalen,
                  int *padlen)
{
  guint8 *pad;
  int padlength;
  int i;

  pad = wrapped_data + wrapped_length - 1;
  padlength = *pad;

  if (padlength > datalen)
    return 1;

  for (i = padlength; i > 0 && *pad == padlength; i--, pad--);
  if (i != 0)
    return 2;

  *padlen = padlength;

  return 0;
}

static int
decrypt_arcfour(gssapi_encrypt_info_t* gssapi_encrypt, guint8 *input_message_buffer, guint8 *output_message_buffer,
                guint8 *key_value, int key_size, int key_type)
{
  guint8 Klocaldata[16];
  int ret;
  int datalen;
  guint8 k6_data[16];
  guint32 SND_SEQ[2];
  guint8 Confounder[8];
  guint8 cksum_data[8];
  int cmp;
  int conf_flag;
  int padlen = 0;
  gcry_cipher_hd_t rc4_handle;
  int i;

  datalen = tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb);

  if(tvb_get_ntohs(gssapi_encrypt->gssapi_wrap_tvb, 4)==0x1000){
    conf_flag=1;
  } else if (tvb_get_ntohs(gssapi_encrypt->gssapi_wrap_tvb, 4)==0xffff){
    conf_flag=0;
  } else {
    return -3;
  }

  if(tvb_get_ntohs(gssapi_encrypt->gssapi_wrap_tvb, 6)!=0xffff){
    return -4;
  }

  ret = arcfour_mic_key(key_value, key_size, key_type,
                        tvb_get_ptr(gssapi_encrypt->gssapi_wrap_tvb, 16, 8),
                        8, /* SGN_CKSUM */
                        k6_data);
  if (ret) {
    return -5;
  }

  tvb_memcpy(gssapi_encrypt->gssapi_wrap_tvb, SND_SEQ, 8, 8);
  if (gcry_cipher_open (&rc4_handle, GCRY_CIPHER_ARCFOUR, GCRY_CIPHER_MODE_STREAM, 0)) {
    return -12;
  }
  if (gcry_cipher_setkey(rc4_handle, k6_data, sizeof(k6_data))) {
    gcry_cipher_close(rc4_handle);
    return -13;
  }
  gcry_cipher_decrypt(rc4_handle, (guint8 *)SND_SEQ, 8, NULL, 0);
  gcry_cipher_close(rc4_handle);

  memset(k6_data, 0, sizeof(k6_data));



  if (SND_SEQ[1] != 0xFFFFFFFF && SND_SEQ[1] != 0x00000000) {
    return -6;
  }


  for (i = 0; i < 16; i++)
    Klocaldata[i] = ((guint8 *)key_value)[i] ^ 0xF0;

  ret = arcfour_mic_key(Klocaldata,sizeof(Klocaldata),key_type,
                        (const guint8 *)SND_SEQ, 4,
                        k6_data);
  memset(Klocaldata, 0, sizeof(Klocaldata));
  if (ret) {
    return -7;
  }

  if(conf_flag) {

    tvb_memcpy(gssapi_encrypt->gssapi_wrap_tvb, Confounder, 24, 8);
    if (gcry_cipher_open (&rc4_handle, GCRY_CIPHER_ARCFOUR, GCRY_CIPHER_MODE_STREAM, 0)) {
      return -14;
    }
    if (gcry_cipher_setkey(rc4_handle, k6_data, sizeof(k6_data))) {
      gcry_cipher_close(rc4_handle);
      return -15;
    }

    gcry_cipher_decrypt(rc4_handle, Confounder, 8, NULL, 0);
    gcry_cipher_decrypt(rc4_handle, output_message_buffer, datalen, input_message_buffer, datalen);
    gcry_cipher_close(rc4_handle);
  } else {
    tvb_memcpy(gssapi_encrypt->gssapi_wrap_tvb, Confounder, 24, 8);
    memcpy(output_message_buffer, input_message_buffer, datalen);
  }
  memset(k6_data, 0, sizeof(k6_data));

  /* only normal (i.e. non DCE style  wrapping use padding ? */
  if(gssapi_encrypt->decrypt_gssapi_tvb==DECRYPT_GSSAPI_NORMAL){
    ret = gssapi_verify_pad(output_message_buffer,datalen,datalen, &padlen);
    if (ret) {
      return -9;
    }
    datalen -= padlen;
  }

  /* don't know what the checksum looks like for dce style gssapi */
  if(gssapi_encrypt->decrypt_gssapi_tvb==DECRYPT_GSSAPI_NORMAL){
    ret = arcfour_mic_cksum(key_value, key_size, KRB5_KU_USAGE_SEAL,
                            cksum_data,
                            tvb_get_ptr(gssapi_encrypt->gssapi_wrap_tvb, 0, 8), 8,
                            Confounder, sizeof(Confounder), output_message_buffer,
                            datalen + padlen);
    if (ret) {
      return -10;
    }

    cmp = tvb_memeql(gssapi_encrypt->gssapi_wrap_tvb, 16, cksum_data, 8); /* SGN_CKSUM */
    if (cmp) {
      return -11;
    }
  }

  return datalen;
}



#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)

static void
decrypt_gssapi_krb_arcfour_wrap(proto_tree *tree _U_, packet_info *pinfo, tvbuff_t *tvb, int keytype, gssapi_encrypt_info_t* gssapi_encrypt)
{
  int ret;
  enc_key_t *ek;
  int length;
  const guint8 *original_data;

  guint8 *cryptocopy=NULL; /* workaround for pre-0.6.1 heimdal bug */
  guint8 *output_message_buffer;

  length=tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb);
  original_data=tvb_get_ptr(gssapi_encrypt->gssapi_encrypted_tvb, 0, length);

  /* don't do anything if we are not attempting to decrypt data */
/*
  if(!krb_decrypt){
    return;
  }
*/
  /* XXX we should only do this for first time, then store somewhere */
  /* XXX We also need to re-read the keytab when the preference changes */

  cryptocopy=(guint8 *)wmem_alloc(wmem_packet_scope(), length);
  output_message_buffer=(guint8 *)wmem_alloc(pinfo->pool, length);

  for(ek=enc_key_list;ek;ek=ek->next){
    /* shortcircuit and bail out if enctypes are not matching */
    if(ek->keytype!=keytype){
      continue;
    }

    /* pre-0.6.1 versions of Heimdal would sometimes change
      the cryptotext data even when the decryption failed.
      This would obviously not work since we iterate over the
      keys. So just give it a copy of the crypto data instead.
      This has been seen for RC4-HMAC blobs.
    */
    memcpy(cryptocopy, original_data, length);
    ret=decrypt_arcfour(gssapi_encrypt,
                        cryptocopy,
                        output_message_buffer,
                        ek->keyvalue,
                        ek->keylength,
                        ek->keytype);
    if (ret >= 0) {
      expert_add_info_format(pinfo, NULL, &ei_spnego_decrypted_keytype,
                             "Decrypted keytype %d in frame %u using %s",
                             ek->keytype, pinfo->num, ek->key_origin);

      gssapi_encrypt->gssapi_decrypted_tvb=tvb_new_child_real_data(tvb, output_message_buffer, ret, ret);
      add_new_data_source(pinfo, gssapi_encrypt->gssapi_decrypted_tvb, "Decrypted GSS-Krb5");
      return;
    }
  }
}

/* borrowed from heimdal */
static int
rrc_rotate(guint8 *data, int len, guint16 rrc, int unrotate)
{
  guint8 *tmp, buf[256];
  size_t left;

  if (len == 0)
    return 0;

  rrc %= len;

  if (rrc == 0)
    return 0;

  left = len - rrc;

  if (rrc <= sizeof(buf)) {
    tmp = buf;
  } else {
    tmp = (guint8 *)g_malloc(rrc);
    if (tmp == NULL)
      return -1;
  }

  if (unrotate) {
    memcpy(tmp, data, rrc);
    memmove(data, data + rrc, left);
    memcpy(data + left, tmp, rrc);
  } else {
    memcpy(tmp, data + left, rrc);
    memmove(data + rrc, data, left);
    memcpy(data, tmp, rrc);
  }

  if (rrc > sizeof(buf))
    g_free(tmp);

  return 0;
}


#define KRB5_KU_USAGE_ACCEPTOR_SEAL     22
#define KRB5_KU_USAGE_ACCEPTOR_SIGN     23
#define KRB5_KU_USAGE_INITIATOR_SEAL    24
#define KRB5_KU_USAGE_INITIATOR_SIGN    25

static void
decrypt_gssapi_krb_cfx_wrap(proto_tree *tree,
                            packet_info *pinfo,
                            tvbuff_t *checksum_tvb,
                            gssapi_encrypt_info_t* gssapi_encrypt,
                            guint16 ec,
                            guint16 rrc,
                            int keytype,
                            unsigned int usage)
{
  guint8 *rotated;
  guint8 *output;
  int datalen;
  tvbuff_t *next_tvb;

  /* don't do anything if we are not attempting to decrypt data */
  if(!krb_decrypt){
    return;
  }

  datalen = tvb_captured_length(checksum_tvb) + tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb);

  rotated = (guint8 *)wmem_alloc(pinfo->pool, datalen);

  tvb_memcpy(checksum_tvb, rotated, 0, tvb_captured_length(checksum_tvb));
  tvb_memcpy(gssapi_encrypt->gssapi_encrypted_tvb, rotated + tvb_captured_length(checksum_tvb),
             0, tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb));

  if (gssapi_encrypt->decrypt_gssapi_tvb==DECRYPT_GSSAPI_DCE) {
    rrc += ec;
  }

  rrc_rotate(rotated, datalen, rrc, TRUE);

  next_tvb=tvb_new_child_real_data(gssapi_encrypt->gssapi_encrypted_tvb, rotated,
                                   datalen, datalen);
  add_new_data_source(pinfo, next_tvb, "GSSAPI CFX");

  output = decrypt_krb5_data(tree, pinfo, usage, next_tvb, keytype, &datalen);

  if (output) {
    guint8 *outdata;

    outdata = (guint8 *)wmem_memdup(pinfo->pool, output, tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb));

    gssapi_encrypt->gssapi_decrypted_tvb=tvb_new_child_real_data(gssapi_encrypt->gssapi_encrypted_tvb,
      outdata,
      tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb),
      tvb_captured_length(gssapi_encrypt->gssapi_encrypted_tvb));
    add_new_data_source(pinfo, gssapi_encrypt->gssapi_decrypted_tvb, "Decrypted GSS-Krb5");
  }
}

#endif /* HAVE_HEIMDAL_KERBEROS || HAVE_MIT_KERBEROS */


#endif

/*
 * This is for GSSAPI Wrap tokens ...
 */
static int
dissect_spnego_krb5_wrap_base(tvbuff_t *tvb, int offset, packet_info *pinfo
#ifndef HAVE_KERBEROS
  _U_
#endif
    , proto_tree *tree, guint16 token_id
#ifndef HAVE_KERBEROS
  _U_
#endif
  , gssapi_encrypt_info_t* gssapi_encrypt
  )
{
  guint16 sgn_alg, seal_alg;
#ifdef HAVE_KERBEROS
  int start_offset=offset;
#endif

  /*
   * The KRB5 blob conforms to RFC1964:
   *   USHORT (0x0102 == GSS_Wrap)
   *   and so on }
   */

  /* Now, the sign and seal algorithms ... */

  sgn_alg = tvb_get_letohs(tvb, offset);
  proto_tree_add_uint(tree, hf_spnego_krb5_sgn_alg, tvb, offset, 2, sgn_alg);

  offset += 2;

  seal_alg = tvb_get_letohs(tvb, offset);
  proto_tree_add_uint(tree, hf_spnego_krb5_seal_alg, tvb, offset, 2, seal_alg);

  offset += 2;

  /* Skip the filler */

  offset += 2;

  /* Encrypted sequence number */

  proto_tree_add_item(tree, hf_spnego_krb5_snd_seq, tvb, offset, 8, ENC_NA);

  offset += 8;

  /* Checksum of plaintext padded data */

  proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset, 8, ENC_NA);

  offset += 8;

  /*
   * At least according to draft-brezak-win2k-krb-rc4-hmac-04,
   * if the signing algorithm is KRB_SGN_ALG_HMAC, there's an
   * extra 8 bytes of "Random confounder" after the checksum.
   * It certainly confounds code expecting all Kerberos 5
   * GSS_Wrap() tokens to look the same....
   */
  if ((sgn_alg == KRB_SGN_ALG_HMAC) ||
      /* there also seems to be a confounder for DES MAC MD5 - certainly seen when using with
         SASL with LDAP between a Java client and Active Directory. If this breaks other things
         we may need to make this an option. gal 17/2/06 */
      (sgn_alg == KRB_SGN_ALG_DES_MAC_MD5)) {
    proto_tree_add_item(tree, hf_spnego_krb5_confounder, tvb, offset, 8, ENC_NA);
    offset += 8;
  }

  /* Is the data encrypted? */
  if (gssapi_encrypt != NULL)
    gssapi_encrypt->gssapi_data_encrypted=(seal_alg!=KRB_SEAL_ALG_NONE);

#ifdef HAVE_KERBEROS
#define GSS_ARCFOUR_WRAP_TOKEN_SIZE 32
  if(gssapi_encrypt && gssapi_encrypt->decrypt_gssapi_tvb){
    /* if the caller did not provide a tvb, then we just use
       whatever is left of our current tvb.
    */
    if(!gssapi_encrypt->gssapi_encrypted_tvb){
      int len;
      len=tvb_reported_length_remaining(tvb,offset);
      if(len>tvb_captured_length_remaining(tvb, offset)){
        /* no point in trying to decrypt,
           we don't have the full pdu.
        */
        return offset;
      }
      gssapi_encrypt->gssapi_encrypted_tvb = tvb_new_subset_length(
          tvb, offset, len);
    }

    /* if this is KRB5 wrapped rc4-hmac */
    if((token_id==KRB_TOKEN_WRAP)
     &&(sgn_alg==KRB_SGN_ALG_HMAC)
     &&(seal_alg==KRB_SEAL_ALG_RC4)){
      /* do we need to create a tvb for the wrapper
         as well ?
      */
      if(!gssapi_encrypt->gssapi_wrap_tvb){
        gssapi_encrypt->gssapi_wrap_tvb = tvb_new_subset_length(
          tvb, start_offset-2,
          GSS_ARCFOUR_WRAP_TOKEN_SIZE);
      }
#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
      decrypt_gssapi_krb_arcfour_wrap(tree,
        pinfo,
        tvb,
        KEYTYPE_ARCFOUR_HMAC,
        gssapi_encrypt);
#endif /* HAVE_HEIMDAL_KERBEROS || HAVE_MIT_KERBEROS */
    }
  }
#endif
  /*
   * Return the offset past the checksum, so that we know where
   * the data we're wrapped around starts.  Also, set the length
   * of our top-level item to that offset, so it doesn't cover
   * the data we're wrapped around.
   *
   * Note that for DCERPC the GSSAPI blobs comes after the data it wraps,
   * not before.
   */
  return offset;
}

/*
 * XXX - This is for GSSAPI GetMIC tokens ...
 */
static int
dissect_spnego_krb5_getmic_base(tvbuff_t *tvb, int offset, packet_info *pinfo _U_, proto_tree *tree)
{
  guint16 sgn_alg;

  /*
   * The KRB5 blob conforms to RFC1964:
   *   USHORT (0x0101 == GSS_GetMIC)
   *   and so on }
   */

  /* Now, the sign algorithm ... */

  sgn_alg = tvb_get_letohs(tvb, offset);
  proto_tree_add_uint(tree, hf_spnego_krb5_sgn_alg, tvb, offset, 2, sgn_alg);

  offset += 2;

  /* Skip the filler */

  offset += 4;

  /* Encrypted sequence number */

  proto_tree_add_item(tree, hf_spnego_krb5_snd_seq, tvb, offset, 8, ENC_NA);

  offset += 8;

  /* Checksum of plaintext padded data */

  proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset, 8, ENC_NA);

  offset += 8;

  /*
   * At least according to draft-brezak-win2k-krb-rc4-hmac-04,
   * if the signing algorithm is KRB_SGN_ALG_HMAC, there's an
   * extra 8 bytes of "Random confounder" after the checksum.
   * It certainly confounds code expecting all Kerberos 5
   * GSS_Wrap() tokens to look the same....
   *
   * The exception is DNS/TSIG where there is no such confounder
   * so we need to test here if there are more bytes in our tvb or not.
   *  -- ronnie
   */
  if (tvb_reported_length_remaining(tvb, offset)) {
    if (sgn_alg == KRB_SGN_ALG_HMAC) {
      proto_tree_add_item(tree, hf_spnego_krb5_confounder, tvb, offset, 8, ENC_NA);

      offset += 8;
    }
  }

  /*
   * Return the offset past the checksum, so that we know where
   * the data we're wrapped around starts.  Also, set the length
   * of our top-level item to that offset, so it doesn't cover
   * the data we're wrapped around.
   */

  return offset;
}

static int
dissect_spnego_krb5_cfx_flags(tvbuff_t *tvb, int offset,
                              proto_tree *spnego_krb5_tree,
                              guint8 cfx_flags _U_)
{
  static const int * flags[] = {
    &hf_spnego_krb5_cfx_flags_04,
    &hf_spnego_krb5_cfx_flags_02,
    &hf_spnego_krb5_cfx_flags_01,
    NULL
  };

  proto_tree_add_bitmask(spnego_krb5_tree, tvb, offset, hf_spnego_krb5_cfx_flags, ett_spnego_krb5_cfx_flags, flags, ENC_NA);
  return (offset + 1);
}

/*
 * This is for GSSAPI CFX Wrap tokens ...
 */
static int
dissect_spnego_krb5_cfx_wrap_base(tvbuff_t *tvb, int offset, packet_info *pinfo
#ifndef HAVE_KERBEROS
  _U_
#endif
  , proto_tree *tree, guint16 token_id _U_
  , gssapi_encrypt_info_t* gssapi_encrypt
  )
{
  guint8 flags;
  guint16 ec;
#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
  guint16 rrc;
#endif
  int checksum_size;
  int start_offset=offset;

  /*
   * The KRB5 blob conforms to RFC4121:
   *   USHORT (0x0504)
   *   and so on }
   */

  /* Now, the sign and seal algorithms ... */

  flags = tvb_get_guint8(tvb, offset);
  offset = dissect_spnego_krb5_cfx_flags(tvb, offset, tree, flags);

  if (gssapi_encrypt != NULL)
    gssapi_encrypt->gssapi_data_encrypted=(flags & 2);

  /* Skip the filler */

  proto_tree_add_item(tree, hf_spnego_krb5_filler, tvb, offset, 1, ENC_NA);
  offset += 1;

  /* EC */
  ec = tvb_get_ntohs(tvb, offset);
  proto_tree_add_item(tree, hf_spnego_krb5_cfx_ec, tvb, offset, 2, ENC_BIG_ENDIAN);
  offset += 2;

  /* RRC */
#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
  rrc = tvb_get_ntohs(tvb, offset);
#endif
  proto_tree_add_item(tree, hf_spnego_krb5_cfx_rrc, tvb, offset, 2, ENC_BIG_ENDIAN);
  offset += 2;

  /* sequence number */

  proto_tree_add_item(tree, hf_spnego_krb5_cfx_seq, tvb, offset, 8, ENC_BIG_ENDIAN);
  offset += 8;

  if (gssapi_encrypt == NULL) /* Probably shoudn't happen, but just protect ourselves */
    return offset;

  /* Checksum of plaintext padded data */

  if (gssapi_encrypt->gssapi_data_encrypted) {
    checksum_size = 44 + ec;

    proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset, checksum_size, ENC_NA);
    offset += checksum_size;

  } else {
    int returned_offset;
    int inner_token_len = 0;

    /*
     * We know we have a wrap token, but we have to let the proto
     * above us decode that, so hand it back in gssapi_wrap_tvb
     * and put the checksum in the tree.
     */

    checksum_size = ec;

    inner_token_len = tvb_reported_length_remaining(tvb, offset);
    if (inner_token_len > ec) {
      inner_token_len -= ec;
    }

    /*
     * We handle only the two common cases for now
     * (rrc == 0 and rrc == ec)
     */
#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
    if (rrc == ec) {
      proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset, checksum_size, ENC_NA);
      offset += checksum_size;
    }
#endif

    returned_offset = offset;
    gssapi_encrypt->gssapi_wrap_tvb = tvb_new_subset_length(tvb, offset,
            inner_token_len);

    offset += inner_token_len;

#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
    if (rrc == 0)
#endif
    {
      proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset, checksum_size, ENC_NA);
    }

    /*
     * Return an offset that puts our caller before the inner
     * token. This is better than before, but we still see the
     * checksum included in the LDAP query at times.
     */
    return returned_offset;
  }

  if(gssapi_encrypt->decrypt_gssapi_tvb){
    /* if the caller did not provide a tvb, then we just use
       whatever is left of our current tvb.
    */
    if(!gssapi_encrypt->gssapi_encrypted_tvb){
      int len;
      len=tvb_reported_length_remaining(tvb,offset);
      if(len>tvb_captured_length_remaining(tvb, offset)){
        /* no point in trying to decrypt,
           we don't have the full pdu.
        */
        return offset;
      }
      gssapi_encrypt->gssapi_encrypted_tvb = tvb_new_subset_length_caplen(
          tvb, offset, len, len);
    }

    if (gssapi_encrypt->gssapi_data_encrypted) {
      /* do we need to create a tvb for the wrapper
         as well ?
      */
      if(!gssapi_encrypt->gssapi_wrap_tvb){
        gssapi_encrypt->gssapi_wrap_tvb = tvb_new_subset_length(
          tvb, start_offset-2,
          offset - (start_offset-2));
      }
    }
  }

#if defined(HAVE_HEIMDAL_KERBEROS) || defined(HAVE_MIT_KERBEROS)
{
  tvbuff_t *checksum_tvb = tvb_new_subset_length(tvb, 16, checksum_size);

  if (gssapi_encrypt->gssapi_data_encrypted) {
    if(gssapi_encrypt->gssapi_encrypted_tvb){
      decrypt_gssapi_krb_cfx_wrap(tree,
        pinfo,
        checksum_tvb,
        gssapi_encrypt,
        ec,
        rrc,
        -1,
        (flags & 0x0001)?
        KRB5_KU_USAGE_ACCEPTOR_SEAL:
        KRB5_KU_USAGE_INITIATOR_SEAL);
    }
  }
}
#endif /* HAVE_HEIMDAL_KERBEROS || HAVE_MIT_KERBEROS */

  /*
   * Return the offset past the checksum, so that we know where
   * the data we're wrapped around starts.  Also, set the length
   * of our top-level item to that offset, so it doesn't cover
   * the data we're wrapped around.
   *
   * Note that for DCERPC the GSSAPI blobs comes after the data it wraps,
   * not before.
   */
  return offset;
}

/*
 * XXX - This is for GSSAPI CFX GetMIC tokens ...
 */
static int
dissect_spnego_krb5_cfx_getmic_base(tvbuff_t *tvb, int offset, packet_info *pinfo _U_, proto_tree *tree)
{
  guint8 flags;
  int checksum_size;

  /*
   * The KRB5 blob conforms to RFC4121:
   *   USHORT (0x0404 == GSS_GetMIC)
   *   and so on }
   */

  flags = tvb_get_guint8(tvb, offset);
  offset = dissect_spnego_krb5_cfx_flags(tvb, offset, tree, flags);

  /* Skip the filler */

  proto_tree_add_item(tree, hf_spnego_krb5_filler, tvb, offset, 5, ENC_NA);
  offset += 5;

  /* sequence number */

  proto_tree_add_item(tree, hf_spnego_krb5_cfx_seq, tvb, offset, 8, ENC_BIG_ENDIAN);
  offset += 8;

  /* Checksum of plaintext padded data */

  checksum_size = tvb_captured_length_remaining(tvb, offset);

  proto_tree_add_item(tree, hf_spnego_krb5_sgn_cksum, tvb, offset,  checksum_size, ENC_NA);
  offset += checksum_size;

  /*
   * Return the offset past the checksum, so that we know where
   * the data we're wrapped around starts.  Also, set the length
   * of our top-level item to that offset, so it doesn't cover
   * the data we're wrapped around.
   */

  return offset;
}

/*
 * XXX - is this for SPNEGO or just GSS-API?
 * RFC 1964 is "The Kerberos Version 5 GSS-API Mechanism"; presumably one
 * can directly designate Kerberos V5 as a mechanism in GSS-API, rather
 * than designating SPNEGO as the mechanism, offering Kerberos V5, and
 * getting it accepted.
 */
static int
dissect_spnego_krb5_wrap(tvbuff_t *tvb, packet_info *pinfo _U_, proto_tree *tree, void *data)
{
  proto_item *item;
  proto_tree *subtree;
  int offset = 0;
  guint16 token_id;
  gssapi_encrypt_info_t* encrypt_info = (gssapi_encrypt_info_t*)data;

  item = proto_tree_add_item(tree, hf_spnego_krb5, tvb, 0, -1, ENC_NA);

  subtree = proto_item_add_subtree(item, ett_spnego_krb5);

  /*
   * The KRB5 blob conforms to RFC1964:
   *   USHORT (0x0102 == GSS_Wrap)
   *   and so on }
   */

  /* First, the token ID ... */

  token_id = tvb_get_letohs(tvb, offset);
  proto_tree_add_uint(subtree, hf_spnego_krb5_tok_id, tvb, offset, 2, token_id);

  offset += 2;

  switch (token_id) {
  case KRB_TOKEN_GETMIC:
    offset = dissect_spnego_krb5_getmic_base(tvb, offset, pinfo, subtree);
    break;

  case KRB_TOKEN_WRAP:
    offset = dissect_spnego_krb5_wrap_base(tvb, offset, pinfo, subtree, token_id, encrypt_info);
    break;

  case KRB_TOKEN_CFX_GETMIC:
    offset = dissect_spnego_krb5_cfx_getmic_base(tvb, offset, pinfo, subtree);
    break;

  case KRB_TOKEN_CFX_WRAP:
    offset = dissect_spnego_krb5_cfx_wrap_base(tvb, offset, pinfo, subtree, token_id, encrypt_info);
    break;

  default:

    break;
  }

  /*
   * Return the offset past the checksum, so that we know where
   * the data we're wrapped around starts.  Also, set the length
   * of our top-level item to that offset, so it doesn't cover
   * the data we're wrapped around.
   */
  proto_item_set_len(item, offset);
  return offset;
}

/* Spnego stuff from here */

static int
dissect_spnego_wrap(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
  proto_item *item;
  proto_tree *subtree;
  int offset = 0;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, TRUE, pinfo);

  MechType_oid = NULL;

  /*
   * We need this later, so lets get it now ...
   * It has to be per-frame as there can be more than one GSS-API
   * negotiation in a conversation.
   */


  item = proto_tree_add_item(tree, proto_spnego, tvb, offset, -1, ENC_NA);

  subtree = proto_item_add_subtree(item, ett_spnego);
  /*
   * The TVB contains a [0] header and a sequence that consists of an
   * object ID and a blob containing the data ...
   * XXX - is this RFC 2743's "Mechanism-Independent Token Format",
   * with the "optional" "use in non-initial tokens" being chosen.
   * ASN1 code addet to spnego.asn to handle this.
   */

  offset = dissect_spnego_InitialContextToken(FALSE, tvb, offset, &asn1_ctx , subtree, -1);

  return offset;
}


static int
dissect_spnego(tvbuff_t *tvb, packet_info *pinfo, proto_tree *parent_tree, void* data _U_)
{
  proto_item *item;
  proto_tree *subtree;
  int offset = 0;
  conversation_t *conversation;
  asn1_ctx_t asn1_ctx;
  asn1_ctx_init(&asn1_ctx, ASN1_ENC_BER, TRUE, pinfo);

  /*
   * We need this later, so lets get it now ...
   * It has to be per-frame as there can be more than one GSS-API
   * negotiation in a conversation.
   */
  next_level_value = (gssapi_oid_value *)p_get_proto_data(wmem_file_scope(), pinfo, proto_spnego, 0);
  if (!next_level_value && !pinfo->fd->flags.visited) {
      /*
       * No handle attached to this frame, but it's the first
       * pass, so it'd be attached to the conversation.
       * If we have a conversation, try to get the handle,
       * and if we get one, attach it to the frame.
       */
      conversation = find_conversation_pinfo(pinfo, 0);

      if (conversation) {
        next_level_value = (gssapi_oid_value *)conversation_get_proto_data(conversation, proto_spnego);
        if (next_level_value)
          p_add_proto_data(wmem_file_scope(), pinfo, proto_spnego, 0, next_level_value);
      }
  }

  item = proto_tree_add_item(parent_tree, proto_spnego, tvb, offset, -1, ENC_NA);

  subtree = proto_item_add_subtree(item, ett_spnego);

  /*
   * The TVB contains a [0] header and a sequence that consists of an
   * object ID and a blob containing the data ...
   * Actually, it contains, according to RFC2478:
   * NegotiationToken ::= CHOICE {
   *          negTokenInit [0] NegTokenInit,
   *          negTokenTarg [1] NegTokenTarg }
   * NegTokenInit ::= SEQUENCE {
   *          mechTypes [0] MechTypeList OPTIONAL,
   *          reqFlags [1] ContextFlags OPTIONAL,
   *          mechToken [2] OCTET STRING OPTIONAL,
   *          mechListMIC [3] OCTET STRING OPTIONAL }
   * NegTokenTarg ::= SEQUENCE {
   *          negResult [0] ENUMERATED {
   *              accept_completed (0),
   *              accept_incomplete (1),
   *              reject (2) } OPTIONAL,
   *          supportedMech [1] MechType OPTIONAL,
   *          responseToken [2] OCTET STRING OPTIONAL,
   *          mechListMIC [3] OCTET STRING OPTIONAL }
   *
   * Windows typically includes mechTypes and mechListMic ('NONE'
   * in the case of NTLMSSP only).
   * It seems to duplicate the responseToken into the mechListMic field
   * as well. Naughty, naughty.
   *
   */
  dissect_spnego_NegotiationToken(FALSE, tvb, offset, &asn1_ctx, subtree, -1);
  return tvb_captured_length(tvb);
}

/*--- proto_register_spnego -------------------------------------------*/
void proto_register_spnego(void) {

  /* List of fields */
  static hf_register_info hf[] = {
    { &hf_spnego_wraptoken,
      { "wrapToken", "spnego.wraptoken",
        FT_NONE, BASE_NONE, NULL, 0x0, "SPNEGO wrapToken",
        HFILL}},
    { &hf_spnego_krb5,
      { "krb5_blob", "spnego.krb5.blob", FT_BYTES,
        BASE_NONE, NULL, 0, NULL, HFILL }},
    { &hf_spnego_krb5_oid,
      { "KRB5 OID", "spnego.krb5_oid", FT_STRING,
        BASE_NONE, NULL, 0, NULL, HFILL }},
    { &hf_spnego_krb5_tok_id,
      { "krb5_tok_id", "spnego.krb5.tok_id", FT_UINT16, BASE_HEX,
        VALS(spnego_krb5_tok_id_vals), 0, "KRB5 Token Id", HFILL}},
    { &hf_spnego_krb5_sgn_alg,
      { "krb5_sgn_alg", "spnego.krb5.sgn_alg", FT_UINT16, BASE_HEX,
        VALS(spnego_krb5_sgn_alg_vals), 0, "KRB5 Signing Algorithm", HFILL}},
    { &hf_spnego_krb5_seal_alg,
      { "krb5_seal_alg", "spnego.krb5.seal_alg", FT_UINT16, BASE_HEX,
        VALS(spnego_krb5_seal_alg_vals), 0, "KRB5 Sealing Algorithm", HFILL}},
    { &hf_spnego_krb5_snd_seq,
      { "krb5_snd_seq", "spnego.krb5.snd_seq", FT_BYTES, BASE_NONE,
        NULL, 0, "KRB5 Encrypted Sequence Number", HFILL}},
    { &hf_spnego_krb5_sgn_cksum,
      { "krb5_sgn_cksum", "spnego.krb5.sgn_cksum", FT_BYTES, BASE_NONE,
        NULL, 0, "KRB5 Data Checksum", HFILL}},
    { &hf_spnego_krb5_confounder,
      { "krb5_confounder", "spnego.krb5.confounder", FT_BYTES, BASE_NONE,
        NULL, 0, "KRB5 Confounder", HFILL}},
    { &hf_spnego_krb5_filler,
      { "krb5_filler", "spnego.krb5.filler", FT_BYTES, BASE_NONE,
        NULL, 0, "KRB5 Filler", HFILL}},
    { &hf_spnego_krb5_cfx_flags,
      { "krb5_cfx_flags", "spnego.krb5.cfx_flags", FT_UINT8, BASE_HEX,
        NULL, 0, "KRB5 CFX Flags", HFILL}},
    { &hf_spnego_krb5_cfx_flags_01,
      { "SendByAcceptor", "spnego.krb5.send_by_acceptor", FT_BOOLEAN, 8,
        TFS (&tfs_set_notset), 0x01, NULL, HFILL}},
    { &hf_spnego_krb5_cfx_flags_02,
      { "Sealed", "spnego.krb5.sealed", FT_BOOLEAN, 8,
        TFS (&tfs_set_notset), 0x02, NULL, HFILL}},
    { &hf_spnego_krb5_cfx_flags_04,
      { "AcceptorSubkey", "spnego.krb5.acceptor_subkey", FT_BOOLEAN, 8,
        TFS (&tfs_set_notset), 0x04, NULL, HFILL}},
    { &hf_spnego_krb5_cfx_ec,
      { "krb5_cfx_ec", "spnego.krb5.cfx_ec", FT_UINT16, BASE_DEC,
        NULL, 0, "KRB5 CFX Extra Count", HFILL}},
    { &hf_spnego_krb5_cfx_rrc,
      { "krb5_cfx_rrc", "spnego.krb5.cfx_rrc", FT_UINT16, BASE_DEC,
        NULL, 0, "KRB5 CFX Right Rotation Count", HFILL}},
    { &hf_spnego_krb5_cfx_seq,
      { "krb5_cfx_seq", "spnego.krb5.cfx_seq", FT_UINT64, BASE_DEC,
        NULL, 0, "KRB5 Sequence Number", HFILL}},


/*--- Included file: packet-spnego-hfarr.c ---*/
#line 1 "./asn1/spnego/packet-spnego-hfarr.c"
    { &hf_spnego_negTokenInit,
      { "negTokenInit", "spnego.negTokenInit_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_negTokenTarg,
      { "negTokenTarg", "spnego.negTokenTarg_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_MechTypeList_item,
      { "MechType", "spnego.MechType",
        FT_OID, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_mechTypes,
      { "mechTypes", "spnego.mechTypes",
        FT_UINT32, BASE_DEC, NULL, 0,
        "MechTypeList", HFILL }},
    { &hf_spnego_reqFlags,
      { "reqFlags", "spnego.reqFlags",
        FT_BYTES, BASE_NONE, NULL, 0,
        "ContextFlags", HFILL }},
    { &hf_spnego_mechToken,
      { "mechToken", "spnego.mechToken",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_mechListMIC,
      { "mechListMIC", "spnego.mechListMIC",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING", HFILL }},
    { &hf_spnego_hintName,
      { "hintName", "spnego.hintName",
        FT_STRING, BASE_NONE, NULL, 0,
        "GeneralString", HFILL }},
    { &hf_spnego_hintAddress,
      { "hintAddress", "spnego.hintAddress",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING", HFILL }},
    { &hf_spnego_mechToken_01,
      { "mechToken", "spnego.mechToken",
        FT_BYTES, BASE_NONE, NULL, 0,
        "OCTET_STRING", HFILL }},
    { &hf_spnego_negHints,
      { "negHints", "spnego.negHints_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_negResult,
      { "negResult", "spnego.negResult",
        FT_UINT32, BASE_DEC, VALS(spnego_T_negResult_vals), 0,
        NULL, HFILL }},
    { &hf_spnego_supportedMech,
      { "supportedMech", "spnego.supportedMech",
        FT_OID, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_responseToken,
      { "responseToken", "spnego.responseToken",
        FT_BYTES, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_thisMech,
      { "thisMech", "spnego.thisMech",
        FT_OID, BASE_NONE, NULL, 0,
        "MechType", HFILL }},
    { &hf_spnego_innerContextToken,
      { "innerContextToken", "spnego.innerContextToken_element",
        FT_NONE, BASE_NONE, NULL, 0,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_delegFlag,
      { "delegFlag", "spnego.delegFlag",
        FT_BOOLEAN, 8, NULL, 0x80,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_mutualFlag,
      { "mutualFlag", "spnego.mutualFlag",
        FT_BOOLEAN, 8, NULL, 0x40,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_replayFlag,
      { "replayFlag", "spnego.replayFlag",
        FT_BOOLEAN, 8, NULL, 0x20,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_sequenceFlag,
      { "sequenceFlag", "spnego.sequenceFlag",
        FT_BOOLEAN, 8, NULL, 0x10,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_anonFlag,
      { "anonFlag", "spnego.anonFlag",
        FT_BOOLEAN, 8, NULL, 0x08,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_confFlag,
      { "confFlag", "spnego.confFlag",
        FT_BOOLEAN, 8, NULL, 0x04,
        NULL, HFILL }},
    { &hf_spnego_ContextFlags_integFlag,
      { "integFlag", "spnego.integFlag",
        FT_BOOLEAN, 8, NULL, 0x02,
        NULL, HFILL }},

/*--- End of included file: packet-spnego-hfarr.c ---*/
#line 1389 "./asn1/spnego/packet-spnego-template.c"
  };

  /* List of subtrees */
  static gint *ett[] = {
    &ett_spnego,
    &ett_spnego_wraptoken,
    &ett_spnego_krb5,
    &ett_spnego_krb5_cfx_flags,


/*--- Included file: packet-spnego-ettarr.c ---*/
#line 1 "./asn1/spnego/packet-spnego-ettarr.c"
    &ett_spnego_NegotiationToken,
    &ett_spnego_MechTypeList,
    &ett_spnego_NegTokenInit,
    &ett_spnego_NegHints,
    &ett_spnego_NegTokenInit2,
    &ett_spnego_ContextFlags,
    &ett_spnego_NegTokenTarg,
    &ett_spnego_InitialContextToken_U,

/*--- End of included file: packet-spnego-ettarr.c ---*/
#line 1399 "./asn1/spnego/packet-spnego-template.c"
  };

  static ei_register_info ei[] = {
    { &ei_spnego_decrypted_keytype, { "spnego.decrypted_keytype", PI_SECURITY, PI_CHAT, "Decryted keytype", EXPFILL }},
    { &ei_spnego_unknown_header, { "spnego.unknown_header", PI_PROTOCOL, PI_WARN, "Unknown header", EXPFILL }},
  };

  expert_module_t* expert_spnego;

  /* Register protocol */
  proto_spnego = proto_register_protocol(PNAME, PSNAME, PFNAME);

  spnego_handle = register_dissector("spnego", dissect_spnego, proto_spnego);

  proto_spnego_krb5 = proto_register_protocol("SPNEGO-KRB5", "SPNEGO-KRB5", "spnego-krb5");

  spnego_krb5_handle = register_dissector("spnego-krb5", dissect_spnego_krb5, proto_spnego_krb5);
  spnego_krb5_wrap_handle = register_dissector("spnego-krb5-wrap", dissect_spnego_krb5_wrap, proto_spnego_krb5);

  /* Register fields and subtrees */
  proto_register_field_array(proto_spnego, hf, array_length(hf));
  proto_register_subtree_array(ett, array_length(ett));
  expert_spnego = expert_register_protocol(proto_spnego);
  expert_register_field_array(expert_spnego, ei, array_length(ei));
}


/*--- proto_reg_handoff_spnego ---------------------------------------*/
void proto_reg_handoff_spnego(void) {

  dissector_handle_t spnego_wrap_handle;

  /* Register protocol with GSS-API module */

  spnego_wrap_handle = create_dissector_handle(dissect_spnego_wrap,  proto_spnego);
  gssapi_init_oid("1.3.6.1.5.5.2", proto_spnego, ett_spnego,
                  spnego_handle, spnego_wrap_handle,
                  "SPNEGO - Simple Protected Negotiation");

  /* Register both the one MS created and the real one */
  /*
   * Thanks to Jean-Baptiste Marchand and Richard B Ward, the
   * mystery of the MS KRB5 OID is cleared up. It was due to a library
   * that did not handle OID components greater than 16 bits, and was
   * fixed in Win2K SP2 as well as WinXP.
   * See the archive of <ietf-krb-wg@anl.gov> for the thread topic
   * SPNEGO implementation issues. 3-Dec-2002.
   */
  gssapi_init_oid("1.2.840.48018.1.2.2", proto_spnego_krb5, ett_spnego_krb5,
                  spnego_krb5_handle, spnego_krb5_wrap_handle,
                  "MS KRB5 - Microsoft Kerberos 5");
  gssapi_init_oid("1.2.840.113554.1.2.2", proto_spnego_krb5, ett_spnego_krb5,
                  spnego_krb5_handle, spnego_krb5_wrap_handle,
                  "KRB5 - Kerberos 5");
  gssapi_init_oid("1.2.840.113554.1.2.2.3", proto_spnego_krb5, ett_spnego_krb5,
                  spnego_krb5_handle, spnego_krb5_wrap_handle,
                  "KRB5 - Kerberos 5 - User to User");

}

/*
 * Editor modelines
 *
 * Local Variables:
 * c-basic-offset: 2
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=2 tabstop=8 expandtab:
 * :indentSize=2:tabSize=8:noTabs=true:
 */
