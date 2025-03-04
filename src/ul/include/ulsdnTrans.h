/**********************************************************************
 * Copyright 1999-2006, Altibase Corporation or its subsidiaries.
 * All rights reserved.
 *********************************************************************/

/**********************************************************************
 * $Id$
 **********************************************************************/

#ifndef _O_ULSDN_TRANS_H_
#define _O_ULSDN_TRANS_H_ 1

#include <ulsdDef.h>

ACI_RC ulsdCallbackShardPrepareResult(cmiProtocolContext *aProtocolContext,
                                      cmiProtocol        *aProtocol,
                                      void               *aServiceSession,
                                      void               *aUserContext);

/* BUG-45967 Data Node의 Shard Session 정리 */
ACI_RC ulsdCallbackShardPrepareResultInternal( void        * aUserContext,
                                               acp_uint8_t   aReadOnly );

ACI_RC ulsdCallbackShardEndPendingTxResult(cmiProtocolContext *aProtocolContext,
                                           cmiProtocol        *aProtocol,
                                           void               *aServiceSession,
                                           void               *aUserContext);

SQLRETURN ulsdShardPrepareTran(ulnDbc       *aDbc,
                               acp_uint32_t  aXIDSize,
                               acp_uint8_t  *aXID,
                               acp_uint8_t  *aReadOnly);

SQLRETURN ulsdShardEndPendingTran(ulnDbc       *aDbc,
                                  acp_uint32_t  aXIDSize,
                                  acp_uint8_t  *aXID,
                                  acp_sint16_t  aCompletionType);

#endif /* _O_ULSDN_TRANS_H_ */
