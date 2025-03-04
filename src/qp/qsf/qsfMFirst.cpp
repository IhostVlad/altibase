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
 * $Id: qsfMFirst.cpp 85090 2019-03-28 01:15:28Z andrew.shin $
 *
 * Description :
 *     PROJ-1075 array type변수의 member function FIRST
 *
 * Syntax :
 *     arr_var.FIRST();
 *     RETURN INTEGER/VARCHAR  <= 첫번째 index를 반환.
 *                                index는 INTEGER/VARCHAR 둘중 하나임
 *
 * Implementation :
 *     1. element가 하나도 없다면 NULL.
 *     2. element들 중 검색하여 MIN 값을 찾아서 넘겨준다.
 *
 **********************************************************************/

#include <qsf.h>
#include <qc.h>
#include <qsvEnv.h>
#include <qcuSqlSourceInfo.h>
#include <qsParseTree.h>
#include <qsvProcVar.h>
#include <qsxArray.h>
#include <qcuSessionPkg.h>

static mtcName qsfMFirstFunctionName[1] = {
    { NULL, 5, (void*)"FIRST" }
};


static IDE_RC qsfMFirstEstimate( mtcNode*     aNode,
                                 mtcTemplate* aTemplate,
                                 mtcStack*    aStack,
                                 SInt         aRemain,
                                 mtcCallBack* aCallBack );

IDE_RC qsfMFirstCalculate(mtcNode*     aNode,
                          mtcStack*    aStack,
                          SInt         aRemain,
                          void*        aInfo,
                          mtcTemplate* aTemplate );

mtfModule qsfMFirstModule = {
    1|MTC_NODE_OPERATOR_FUNCTION|MTC_NODE_VARIABLE_TRUE,
    ~(MTC_NODE_INDEX_MASK),
    1.0,  // default selectivity (비교 연산자가 아님)
    qsfMFirstFunctionName,
    NULL,
    mtf::initializeDefault,
    mtf::finalizeDefault,
    qsfMFirstEstimate
};

const mtcExecute qsfExecute = {
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    mtf::calculateNA,
    qsfMFirstCalculate,
    NULL,
    mtx::calculateNA,
    mtk::estimateRangeNA,
    mtk::extractRangeNA
};

IDE_RC qsfMFirstEstimate( mtcNode*     aNode,
                          mtcTemplate* aTemplate,
                          mtcStack*    aStack,
                          SInt      /* aRemain */,
                          mtcCallBack* aCallBack )
{
/***********************************************************************
 *
 * Description : PROJ-1075 first함수의 estimate
 *
 * Implementation :
 *            기본적인 routine은 일반 qsf~함수들과 같으나,
 *            host variable binding을 허용하지 않고
 *            psm내부에서만 사용이 가능하다.
 *
 *            다음과 같은 유형으로 들어올 수 있다.
 *            (1) var_name.first()
 *            (2) label_name.var_name.first()
 *            var_name은 qtcNode의 tableName에 해당되므로 무조건 존재해야 한다.
 *            qtcNode->userName, tableName을 이용하여 array type variable을 검색.
 *            execute->calculateInfo에 찾은 변수의 정보를 연결하여 준다.
 *            return value의 유형은 해당 변수의 key column type과 동일하게 생성.
 *
 ***********************************************************************/    

    qcStatement     * sStatement;
    qtcNode         * sNode;
    qsVariables     * sArrayVariable;
    idBool            sIsFound;
    qcuSqlSourceInfo  sqlInfo;
    qtcColumnInfo   * sColumnInfo;
    mtcColumn       * sKeyColumn;
    UInt              sArgumentCount;

    sStatement      = ((qcTemplate*)aTemplate)->stmt;

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_QUANTIFIER_MASK ) ==
                    MTC_NODE_QUANTIFIER_TRUE,
                    ERR_NOT_AGGREGATION );

    IDE_TEST_RAISE( ( aNode->lflag & MTC_NODE_ARGUMENT_COUNT_MASK ) != 0,
                    ERR_INVALID_FUNCTION_ARGUMENT );

    sNode            = (qtcNode*)aNode;

    aStack[0].column = aTemplate->rows[aNode->table].columns + aNode->column;

    // 적합성 검사. tableName은 반드시 존재해야 함.
    IDE_DASSERT( QC_IS_NULL_NAME(sNode->tableName) == ID_FALSE );

    // array type 변수를 검색.
    IDE_TEST( qsvProcVar::searchArrayVar( sStatement,
                                          sNode,
                                          &sIsFound,
                                          &sArrayVariable )
              != IDE_SUCCESS );

    if( sIsFound == ID_FALSE )
    {
        sqlInfo.setSourceInfo(
            sStatement,
            &sNode->tableName );
        IDE_RAISE(ERR_NOT_FOUND_VAR);
    }
    else
    {
        aTemplate->rows[aNode->table].execute[aNode->column] = qsfExecute;

        // 변수의 table, column정보를 execute->calculateInfo에 연결한다.
        IDU_FIT_POINT( "qsfMFirst::qsfMFirstEstimate::alloc::ColumnInfo" );
        IDE_TEST( aCallBack->alloc( aCallBack->info,
                                    ID_SIZEOF( qtcColumnInfo ),
                                    (void**)&sColumnInfo )
                  != IDE_SUCCESS );

        sColumnInfo->table    = sArrayVariable->common.table;
        sColumnInfo->column   = sArrayVariable->common.column;
        sColumnInfo->objectId = sArrayVariable->common.objectID;

        aTemplate->rows[aNode->table].execute[aNode->column].calculateInfo = (void*)sColumnInfo;

        sKeyColumn = sArrayVariable->typeInfo->columns->basicInfo;

        sArgumentCount = sKeyColumn->flag & MTC_COLUMN_ARGUMENT_COUNT_MASK;

        // return값은 keyColumn을 따라감.
        IDE_TEST( mtc::initializeColumn( aStack[0].column,
                                         sKeyColumn->module,
                                         sArgumentCount,
                                         sKeyColumn->precision,
                                         sKeyColumn->scale )
                  != IDE_SUCCESS );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_NOT_AGGREGATION );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_NOT_AGGREGATION));

    IDE_EXCEPTION( ERR_INVALID_FUNCTION_ARGUMENT );
    IDE_SET(ideSetErrorCode(mtERR_ABORT_INVALID_FUNCTION_ARGUMENT));

    IDE_EXCEPTION(ERR_NOT_FOUND_VAR);
    {
        (void)sqlInfo.init(sStatement->qmeMem);
        IDE_SET(
            ideSetErrorCode(qpERR_ABORT_QSV_NOT_EXIST_VARIABLE_NAME_SQLTEXT,
                            sqlInfo.getErrMessage() ));
        (void)sqlInfo.fini();
    }

    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}


IDE_RC qsfMFirstCalculate(mtcNode*     aNode,
                          mtcStack*    aStack,
                          SInt         aRemain,
                          void*        aInfo,
                          mtcTemplate* aTemplate )
{
/***********************************************************************
 *
 * Description : PROJ-1075 first함수의 calculate
 *
 * Implementation :
 *          aInfo에서 qsxArrayInfo를 가져와서
 *          searchFirst함수를 호출한다.
 *          단, 데이터가 없을 시에도 정책상 null을 올려보낸다.
 *
 ***********************************************************************/    

    qsxArrayInfo   * sArrayInfo;
    mtcColumn      * sArrayColumn;
    qtcColumnInfo  * sColumnInfo;
    void           * sFirstKey;
    idBool           sFound;
    UInt             sActualSize;
    mtcTemplate    * sTemplateForArrayVar;

    sColumnInfo = (qtcColumnInfo*)aInfo;

    IDE_TEST( mtf::postfixCalculate( aNode,
                                     aStack,
                                     aRemain,
                                     aInfo,
                                     aTemplate )
              != IDE_SUCCESS );

    /* BUG-38243
       array method 사용 시, 해당 array는 해당 aTemplate이 아닌
       다른 template에 정보가 있을 수 있다. */
    if ( sColumnInfo->objectId == QS_EMPTY_OID )
    {
        sTemplateForArrayVar = aTemplate;
    }
    else
    {
        IDE_TEST( qcuSessionPkg::getTmplate( ((qcTemplate *)aTemplate)->stmt,
                                             sColumnInfo->objectId,
                                             aStack,
                                             aRemain,
                                             &sTemplateForArrayVar )
                  != IDE_SUCCESS );

        IDE_DASSERT( sTemplateForArrayVar != NULL );
    }

    sArrayColumn = sTemplateForArrayVar->rows[sColumnInfo->table].columns + sColumnInfo->column;

    // PROJ-1904 Extend UDT
    sArrayInfo = *((qsxArrayInfo ** )( (UChar*) sTemplateForArrayVar->rows[sColumnInfo->table].row
                                    + sArrayColumn->column.offset ));

    // 적합성 검사.
    IDE_TEST_RAISE( sArrayInfo == NULL, ERR_INVALID_ARRAY );

    IDE_TEST( qsxArray::searchFirst( sArrayInfo,
                                     &sFirstKey,
                                     &sFound )
              != IDE_SUCCESS );

    if( sFound == ID_TRUE )
    {
        sActualSize = aStack[0].column->module->actualSize(
            aStack[0].column,
            sFirstKey );
        // module, precision, scale모두 estimate시 동일하게 맞춰놓았으므로
        // actualSize를 구할 때 aStack[0].column->module을 사용한다.
        idlOS::memcpy( aStack[0].value,
                       sFirstKey,
                       sActualSize );
    }
    else
    {
        aStack[0].column->module->null( aStack[0].column,
                                        aStack[0].value );
    }

    return IDE_SUCCESS;

    IDE_EXCEPTION( ERR_INVALID_ARRAY );
    {
        IDE_SET(ideSetErrorCode(qpERR_ABORT_QSX_UNINITIALIZED_ARRAY));
    }
    IDE_EXCEPTION_END;

    return IDE_FAILURE;
}
