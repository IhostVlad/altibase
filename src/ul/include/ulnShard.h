/**
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _O_ULN_SHARD_H_
#define _O_ULN_SHARD_H_ 1

void ulnShardDbcContextInitialize(ulnDbc *aDbc);

#ifdef COMPILE_SHARDCLI /* BUG-46092 */
void ulnShardDbcContextFinalize( ulnDbc *aDbc );
#endif /* COMPILE_SHARDCLI */

void ulnShardStmtContextInitialize(ulnStmt *aStmt);

#endif /* _O_ULN_SHARD_H_ */
