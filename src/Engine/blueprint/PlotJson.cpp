/*
 * ======================= plotJson.cpp =======================
 *                          -- tpr --
 *                                        CREATE -- 2019.12.03
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 * handle plot-blueprint json files
 */
#include "PlotBlueprint.h"


//--------------- Libs ------------------//
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "tprGeneral.h"

//--------------- Engine ------------------//
#include "global.h"
#include "fileIO.h"
#include "tprCast.h"
#include "blueprint_oth.h"

#include "GoSpecFromJson.h"

#include "json_oth.h"

#include "esrc_state.h"

//--------------- Script ------------------//



using namespace rapidjson;

#include <iostream>
using std::cout;
using std::endl;


namespace blueprint {//------------------ namespace: blueprint start ---------------------//
namespace plotJson_inn {//-------- namespace: plotJson_inn --------------//

    void parse_single_plotJsonFile( const std::string &path_file_ );

}//------------- namespace: plotJson_inn end --------------//


void parse_plotJsonFiles(){

    cout << "   ----- parse_plotJsonFiles: start ----- " << endl;


    esrc::is_setState("json_gameObj"); // MUST


    std::vector<std::string> path_files {};
    json::collect_fileNames( path_blueprintDatas, "plots", "_files.json", path_files );
    //---
    for( const auto &i : path_files ){
        plotJson_inn::parse_single_plotJsonFile(i);
    }

    cout << "   ----- parse_plotJsonFiles: end ----- " << endl;
}



namespace plotJson_inn {//-------- namespace: plotJson_inn --------------//




void parse_single_plotJsonFile( const std::string &path_file_ ){
    //-----------------------------//
    //         load file
    //-----------------------------//
    auto jsonBufUPtr = read_a_file( path_file_ );

    //-----------------------------//
    //      parce JSON data
    //-----------------------------//
    Document doc;
    doc.Parse( jsonBufUPtr->c_str() );

    std::string plotName {};
    std::string pngPath_M {};
    IntVec2 frameNum {};
    size_t  totalFrameNum {};
    
    
    //=====//
    tprAssert( doc.IsArray() );
    for( auto &docEnt : doc.GetArray() ){


        {//--- plotName ---//
            const auto &a = json::check_and_get_value( docEnt, "plotName", json::JsonValType::String );
            plotName = a.GetString();
        }

        //--- plot ---
        auto plotId = PlotBlueprint::init_new_plot( plotName );
        auto &plotRef = PlotBlueprint::get_plotBlueprintRef( plotId );

        {//--- pngLPath ---//
            const auto &a = json::check_and_get_value( docEnt, "pngLPath", json::JsonValType::String );
            std::string pngLPath = a.GetString();
            std::string headPath = tprGeneral::path_combine(path_blueprintDatas, "plots");
            pngPath_M = tprGeneral::path_combine( headPath, pngLPath );
        }
        {//--- frameNum.col ---//
            const auto &a = json::check_and_get_value( docEnt, "frameNum.col", json::JsonValType::Int );
            frameNum.x =  a.GetInt();
        }
        {//--- frameNum.row ---//
            const auto &a = json::check_and_get_value( docEnt, "frameNum.row", json::JsonValType::Int );
            frameNum.y =  a.GetInt();
        }
        {//--- totalFrameNum ---//
            const auto &a = json::check_and_get_value( docEnt, "totalFrameNum", json::JsonValType::Uint64 );
            totalFrameNum =  cast_2_size_t(a.GetUint64());
            tprAssert( totalFrameNum <= cast_2_size_t(frameNum.x * frameNum.y) );
        }

        // 读取解析 png 数据，
        IntVec2 frameSizeByMapEnt = parse_png( plotRef.getnc_mapDatasRef(), pngPath_M, frameNum, totalFrameNum, BlueprintType::Plot );
        plotRef.set_sizeByMapEnt( frameSizeByMapEnt ); // 不一定必须是正方形


        //--- varTypes ---//
        const auto &varTypes = json::check_and_get_value( docEnt, "varTypes", json::JsonValType::Array );
        for( auto &varType : varTypes.GetArray() ){
            tprAssert( varType.IsObject() );

            VariableTypeIdx varTypeIdx {};
            std::unique_ptr<VarTypeDatas_Plot> varTypeDatasUPtr = std::make_unique<VarTypeDatas_Plot>();

            {//--- type ---//
                const auto &a = json::check_and_get_value( varType, "type", json::JsonValType::String );
                varTypeIdx = str_2_variableTypeIdx( a.GetString() );
            }
            {//--- isAllInstanceUseSamePlan ---//
                const auto &a = json::check_and_get_value( varType, "isAllInstanceUseSamePlan", json::JsonValType::Bool );
                varTypeDatasUPtr->set_isAllInstanceUseSamePlan( a.GetBool() );

            }
            {//--- goSpecPool ---//
                std::string MultiGoMeshType {};
                size_t num  {};

                //---
                const auto &goSpecPool = json::check_and_get_value( varType, "goSpecPool", json::JsonValType::Array );
                for( auto &ent : goSpecPool.GetArray() ){

                    {//--- num ---//
                        const auto &a = json::check_and_get_value( ent, "num", json::JsonValType::Uint64 );
                        num = cast_2_size_t( a.GetUint64() );
                    }

                    // 允许写入 0
                    if( num == 0 ){
                        continue;
                    }

                    std::unique_ptr<GoSpec> goSpecUPtr = std::make_unique<GoSpec>();

                    {//--- goSpeciesName ---//
                        const auto &a = json::check_and_get_value( ent, "goSpeciesName", json::JsonValType::String );
                        goSpecUPtr->goSpeciesId = GoSpecFromJson::str_2_goSpeciesId( a.GetString() );
                    }

                    //--- MultiGoMeshType ---//
                    if( ent.HasMember("MultiGoMeshType") ){
                        goSpecUPtr->isMultiGoMesh = true;
                        const auto &a = json::check_and_get_value( ent, "MultiGoMeshType", json::JsonValType::String );
                        goSpecUPtr->multiGoMeshType = MultiGoMesh::str_2_multiGoMeshTypeId( a.GetString() );

                    }else{
                        goSpecUPtr->isMultiGoMesh = false;
                    
                        {//--- afsName ---//
                            const auto &a = json::check_and_get_value( ent, "afsName", json::JsonValType::String );
                            std::string afsName = a.GetString();
                            tprAssert( GoSpecFromJson::is_find_in_afsNames(goSpecUPtr->goSpeciesId, afsName) );
                            goSpecUPtr->afsName = afsName;
                        }
                        {//--- animLabel ---//
                            const auto &a = json::check_and_get_value( ent, "animLabel", json::JsonValType::String );
                            goSpecUPtr->animLabel = str_2_AnimLabel(a.GetString());
                        }
                    }

                    //-- goSpecUPtr 创建完毕 --
                    varTypeDatasUPtr->insert_2_goSpecPool( std::move(goSpecUPtr), num );
                }
            }

            varTypeDatasUPtr->init_check();
            plotRef.insert_2_varTypeDatas( varTypeIdx, std::move(varTypeDatasUPtr) );
        }

        plotRef.init_check();

    }
}





}//------------- namespace: plotJson_inn end --------------//
}//--------------------- namespace: blueprint end ------------------------//
