/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 

/***********************************************************************
 * $Id: qsfDeleting.cpp 85090 2019-03-28 01:15:28Z andrew.shin $
 **********************************************************************/

#include <qsf.h>
#include <qci.h>
#include <qcg.h>
#include <qsxEnv.h>

extern mtdModule mtdBoolean;

static mtcName qsfDeletingFunctionName[1] = {
    { NULL, 9, (void*)"_DELETING" }
};

static IDE_RC qsfDeletingEstimate( mtcNode*     aNode,
                                 mtcTemplate* aTemplate,
                                 mtcStack*    aStack,
                                 SInt         aRemain,
                                 mtcCallBack* aCallBack );

IDE_RC qsfDeletingCalculate(mtcNode*     aNode,
                          mtcStack*    aStack,
                          SInt         aRemain,
                          void*        aInfo,
                          mtcTemplate* aTemplate );

mtfModule qsfDeletingModule = {
    1|MTC_NODE_OPERATOR_FUNCTION|MTC_NODE_VARIABLE_TRUE,
    ~0,
    1.0,  // default selectivity (비교 연산자가 아님)
    qsfDeletingFunctionName,
    NULL,
    mtf::initializeDefault,
    mtf::finalizeDefault,
    qsfDeletingEstimate
};

const mtcExecute qsfExecute = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    qsfDeletingCalculate,
    NULL,
    mtx::calculateNA,
    mtk::estimateRangeNA,
    mtk::extractRangeNA
};

IDE_RC qsfDeletingEstimate( mtcNode*  aNode,
                          mtcTemplate* aTemplate,
                          mtcStack*    aStack,
                          SInt         /* aRemain */,
                          mtcCallBack* /* aCallBack */)
{
    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_QUANTIFIER_MASK ) ==
                    MTC_NODE_QUANTIFIER_TRUE,
                    ERR_NOT_AGGREGATION );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_ARGUMENT_COUNT_MASK ) != 0,
                    ERR_INVALID_FUNCTION_ARGUMENT );

    aStack[0].column = aTemplate->rows[aNode->table].columns + aNode->column;

    aTemplate->rows[aNode->table].execute[aNode->column] = qsfExecute;

    IDE_TEST( mtc::initializeColumn( aStack[0].column,
                                     &mtdBoolean,
                                     0,
                                     0,
                                     0 )
              != IDE_SUCCESS );

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_AGGREGATION );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_NOT_AGGREGATION));

    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}

IDE_RC qsfDeletingCalculate(mtcNode*     aNode,
                          mtcStack*    aStack,
                          SInt         aRemain,
                          void*        aInfo,
                          mtcTemplate* aTemplate )
{
    qcStatement    * sStatement;
    mtdBooleanType * sReturnValue;
    
    IDE_TEST( mtf::postfixCalculate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate )
              != IDE_SUCCESS );

    sStatement = ((qcTemplate*)aTemplate)->stmt;

    sReturnValue = (mtdBooleanType*)aStack[0].value;

    if ( sStatement->spxEnv->mTriggerEventType == QCM_TRIGGER_EVENT_DELETE )
    {
        *sReturnValue = MTD_BOOLEAN_TRUE;
    }
    else
    {
        *sReturnValue = MTD_BOOLEAN_FALSE;
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION_END;
    
    return IDE_FAILURE;
}
