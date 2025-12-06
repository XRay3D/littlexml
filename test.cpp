#ifndef LXML_USE_MODULES
#include "lxml.hpp"
#else
import lxml;
#endif

/*
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2 — копия.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2.fst
/home/x-ray/Документы/TopoR/Examples/Example_04/Arcs.fst
/home/x-ray/Документы/TopoR/Examples/Example_04/Arcs_standard_routing.fst
/home/x-ray/Nextcloud/HARTMASTER/B0505XT-1WR2.fst
/home/x-ray/Nextcloud/HARTMASTER/HARTMASTER.fst
/home/x-ray/Nextcloud/HARTMASTER/HARTMASTER2.fst
/home/x-ray/Документы/TopoR/Examples/Example_05/MinVia.fst
/home/x-ray/Документы/TopoR/Examples/Example_05/MinVia_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement.fstout.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement_standard_routing.fstout.fst
/home/x-ray/Документы/TopoR/Examples/Example_06/Signals.fst
/home/x-ray/Документы/TopoR/Examples/Example_06/Signals.fstout.fst
/home/x-ray/Документы/TopoR/Examples/Example_01/SingleLayer.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_2L.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_4L.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_standard_routing.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/edited.fst


/home/x-ray/Nextcloud/HARTMASTER/B0505XT-1WR2.fst
/home/x-ray/Документы/TopoR/Examples/Example_01/SingleLayer.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2_PNP.fst
/home/x-ray/Документы/TopoR/Examples/Example_06/Signals.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_05/MinVia.fst
/home/x-ray/Документы/TopoR/Examples/Example_05/MinVia_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_04/Arcs_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_02/Placement.fst
/home/x-ray/Документы/TopoR/Examples/Example_04/Arcs.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_4L.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK-310_Soft/AMK_TESTER/AMK_TESTER_ASCII.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_standard_routing.fst
/home/x-ray/Документы/TopoR/Examples/Example_03/arz_2L.fst
/home/x-ray/Nextcloud/HARTMASTER/HARTMASTER.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V1.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2_POS.fst
/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2.fst

 test.xml
*/
int main() {

    std::string_view file = "/home/x-ray/Nextcloud/SRC/AMK-310/AMK310_MCU/AMK310_MCU_V2.fst";
    XML::Document doc;
    if(doc.load(file)) {
        XML::Node& str = *doc.root[0];

        XML::NodeList fields = str.children("field");
        for(int i = 0; i < fields.size(); i++) {
            XML::Node* field = fields[i];
            XML::Attribute& type = field->attr("type");
            type.value = __TIME__;
        }

        doc.write("out.xml", 4);
    }
    //--palette='ad=1;3;38;5;154:de=1;3;38;5;9'
    // system("diff --color -b -B -u /home/x-ray/Nextcloud/HARTMASTER/B0505XT-1WR2.fst out.xml");
    system(std::format("diff --color -b -B -u {} out.xml", file).c_str());
    // system(std::format("kdiff3 {} out.xml", file).c_str());

    return 0;
}
