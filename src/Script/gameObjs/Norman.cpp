/*
 * ========================= Norman.cpp ==========================
 *                          -- tpr --
 *                                        创建 -- 2019.01.30
 *                                        修改 -- 2019.01.30
 * ----------------------------------------------------------
 * 
 * ----------------------------
 */
#include "Script/gameObjs/Norman.h"

//-------------------- C --------------------//
#include <cassert> //- assert

//-------------------- CPP --------------------//
#include <functional>
#include <string>

//-------------------- Engine --------------------//
#include "srcs_engine.h" 

//-------------------- Script --------------------//
#include "Script/actionHandle/Cycle.h"
#include "Script/resource/srcs_script.h" 

using namespace std::placeholders;

#include "debug.h" 


namespace gameObjs{//------------- namespace gameObjs ----------------


namespace{//-------------- namespace ------------------//
}//------------------ namespace: end ------------------//


/* ===========================================================
 *                         init
 * -----------------------------------------------------------
 */
void Norman::init( GameObj *_goPtr ){

    assert( _goPtr != nullptr );
    goPtr = _goPtr;

    //-------- bind callback funcs ---------//
    //-- 故意将 首参数this 绑定到 保留类实例 dog_a 身上
    goPtr->RenderUpdate = std::bind( &Norman::RenderUpdate, &norman, _1 );   
    goPtr->LogicUpdate  = std::bind( &Norman::LogicUpdate,  &norman, _1 );
    goPtr->BeAffect     = std::bind( &Norman::BeAffect,     &norman, _1 ); 

    //-------- go self vals ---------//
    goPtr->species = Norman::specId;
    goPtr->family = GameObjFamily::Major;
    goPtr->parentId = NULLID;
    goPtr->state = GameObjState::Waked;
    goPtr->moveState = GameObjMoveState::Movable;
    goPtr->weight = 5.0f;

    goPtr->isTopGo = true;
    goPtr->isActive = true;
    goPtr->isDirty = false;
    goPtr->isControlByPlayer = false;

    goPtr->move.set_speedLv( SpeedLevel::LV_6 );
    goPtr->move.set_MoveType( true ); //- tmp

    goPtr->goPos.set_alti( 0.0f );

    //-------- action／actionHandle/ goMesh ---------//

        //-- 制作唯一的 mesh 实例: "root" --
        GameObjMesh &goMeshRef = goPtr->creat_new_goMesh( "root" );
        goMeshRef.init( goPtr ); 
        goMeshRef.picMesh.set_shader_program( &esrc::rect_shader );
        goMeshRef.shadowMesh.set_shader_program( &esrc::rect_shader );
        goMeshRef.isVisible = true;
        goMeshRef.isCollide = true;
        //-- bind action / actionHandle --
        goMeshRef.bind_action( "norman" );
        actionHdle::cycle_obj.bind( goMeshRef.get_actionHandlePtr(), 
                                    6,                //- 起始图元帧序号
                                    11,                //- 结束图元帧序号
                                    6,               //- 入口图元帧序号
                                    std::vector<int>{ 6, 4, 4, 6, 4, 4 }, //- steps
                                    false,            //- isStepEqual
                                    true              //- isOrder
                                    ); 

        goMeshRef.isVisible = true;
        goMeshRef.isCollide = true;
        goMeshRef.isFlipOver = false;

        //-- goMesh pos in go --
        goMeshRef.pposOff = glm::vec2{ 0.0f, 0.0f }; //- 此 goMesh 在 go 中的 坐标偏移 
        goMeshRef.off_z = 0.0f;  //- 作为 0号goMesh,此值必须为0

    //-------- go.binary ---------//
    goPtr->resize_binary( sizeof(Norman_Binary) );
    bp = (Norman_Binary*)goPtr->get_binaryHeadPtr(); //- 绑定到本地指针

    bp->HP = 100;
    //...
}

/* ===========================================================
 *                       bind
 * -----------------------------------------------------------
 * -- 在 “工厂”模式中，将本具象go实例，与 一个已经存在的 go实例 绑定。
 * -- 这个 go实例 的类型，应该和 本类一致。
 */
void Norman::bind( GameObj *_goPtr ){
}


/* ===========================================================
 *                       rebind
 * -----------------------------------------------------------
 * -- 从硬盘读取到 go实例数据后，重bind callback
 * -- 会被 脚本层的一个 巨型分配函数 调用
 */
void Norman::rebind( GameObj *_goPtr ){
}

/* ===========================================================
 *                      RenderUpdate
 * -----------------------------------------------------------
 */
void Norman::RenderUpdate( GameObj *_goPtr ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    assert( _goPtr->species == Norman::specId );
    //-- rebind ptr -----
    goPtr = _goPtr;
    bp = (Norman_Binary*)goPtr->get_binaryHeadPtr();

    //=====================================//
    //           test: AI
    //-------------------------------------//
    //...

    //=====================================//
    //         更新 位移系统
    //-------------------------------------//
    goPtr->move.RenderUpdate();

    //=====================================//
    //  将 确认要渲染的 goMeshs，添加到 renderPool         
    //-------------------------------------//
    auto ipair = goPtr->goMeshs.begin();
    for( ; ipair!=goPtr->goMeshs.end(); ipair++ ){
        GameObjMesh &goMeshRef = ipair->second;

        //-- 也许不该放在 这个位置 --
        if( goMeshRef.isVisible == false ){
            continue;
        }

        //=== 传参到 scriptBuf : [无参数] ===
        goMeshRef.get_actionHandle_func("update")( goMeshRef.get_actionHandlePtr(), 0);
        //=== 从 scriptBuf 取返回值 : [无返回值] ===

        goMeshRef.shadowMesh.refresh_translate();
        goMeshRef.shadowMesh.refresh_scale_auto(); //- 没必要每帧都执行
        goMeshRef.picMesh.refresh_translate();
        goMeshRef.picMesh.refresh_scale_auto(); //- 没必要每帧都执行

        esrc::renderPool_goMeshs_pic.insert({ goMeshRef.shadowMesh.get_render_z(), (ChildMesh*)&(goMeshRef.shadowMesh) });
        esrc::renderPool_goMeshs_shadow.push_back( (ChildMesh*)&(goMeshRef.picMesh) );
    }

}


/* ===========================================================
 *                        LogicUpdate
 * -----------------------------------------------------------
 */
void Norman::LogicUpdate( GameObj *_goPtr ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    assert( _goPtr->species == Norman::specId );
    //-- rebind ptr -----
    goPtr = _goPtr;
    bp = (Norman_Binary*)goPtr->get_binaryHeadPtr();
    //=====================================//

    // 什么也没做...
}



/* ===========================================================
 *                       BeAffect
 * -----------------------------------------------------------
 */
void Norman::BeAffect( GameObj *_goPtr ){
}




}//------------- namespace gameObjs: end ----------------

