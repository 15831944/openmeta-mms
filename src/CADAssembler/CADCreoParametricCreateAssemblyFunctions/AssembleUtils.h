#ifndef ASSEMBLE_UTILS_H
#define ASSEMBLE_UTILS_H
#include <cc_CommonStructures.h>
#include <cc_CommonFunctions.h>
#include <isis_application_exception.h>
#include <isis_include_ptc_headers.h>
#include <fstream>
#include <map>
#include <unordered_map>
#include "cc_CadFactoryAbstract.h"

#include <boost/filesystem.hpp>

namespace isis
{
	std::string META_PATH();









	void RetrieveComputationOfAGivenType( const std::list<CADComputation>	&in_AssemblyMetrics,
										  e_ComputationType					in_ComputationType,
										  std::vector<CADComputation>		&out_CADComputations );


	void SetupLogFile( const std::string in_LogFileName, std::ofstream &in_out_LogFile ) throw (isis::application_exception);

	// This function hase a side effect, it changed the current working directory, and the input parameter.
	::boost::filesystem::path SetupWorkingDirectory( std::string & inout_workingDirectory );

	const std::string manufacturingManifestJson_PathAndFileName = ".\\manufacturing.manifest.json";

	void ProAsmcomppathToList(std::list<int> &out_list, const ProAsmcomppath &in_path);

    ProError Retrieve_ProAsmcomppath(const ProSolid in_assembly_model, const std::list<int> in_base_model_path_list, ProAsmcomppath &out_base_comp_path);
    
	void     Retrieve_ProAsmcomppath_WithExceptions(	const ProSolid in_assembly_model, 
														const std::list<int> &in_base_model_path_list, 
														ProAsmcomppath &out_base_comp_path)  throw (isis::application_exception);
	
	void Populate_c_id_table( const std::list<int> &in_path_list, ProIdTable out_c_id_table, int &out_c_id_table_size );

	void RetrieveTranformationMatrix_Assembly_to_Child (  
							const std::string  &in_AssemblyComponentID,
							const std::list<int>	   &in_ChildComponentPaths,
							std::map<std::string, isis::CADComponentData>		&in_CADComponentData_map,  
							ProBoolean   in_bottom_up,
							double out_TransformationMatrix[4][4] )  throw (isis::application_exception);

	void RetrieveTranformationMatrix_Assembly_to_Child (  
							const ProSolid	   &in_assembly_model,
							const std::list<int>	   &in_ChildComponentPaths, 
							ProBoolean   in_bottom_up,
							double out_TransformationMatrix[4][4] )  throw (isis::application_exception);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const std::string	CouldNotFindManufacturingManifestError =
			"\nINFORMATION: Did not update manufacturing manifest file (" +
			isis::manufacturingManifestJson_PathAndFileName + ") with " + 
			"\nthe mapping of component-instance-ID to STEP-file-name because the manifest file " +
			"\ncould not be found.  The manufacturing manifest file would only exist if the " +
			"\nCyPhyPrepareIFab interpreter had been invoked. Typically, CyPhyPrepareIFab " +
			"\nwould not have been invoked."; 

	const std::string	NotUpdatingManufacturingManifest_SeparateSTEPFilesNotRequested =
			"\nINFORMATION: Did not update manufacturing manifest file (" +
			isis::manufacturingManifestJson_PathAndFileName + ") for the" +
			"\nassembled parts because separate STEP part files were not requested." +
			"\nAP203_E2_SEPARATE_PART_FILES or AP214_SEPARATE_PART_FILES must be requested" +
			"\nin order for the manufacturing manifest to be updated for assembled parts.  The " +
			"\nmanifest will be updated for any unassembled parts.";


 
	void UpdateManufacturingManifestWithSTEPFileInfo( 
									e_DataExchangeVersion in_DataExchangeVersion, // AP203_SINGLE_FILE, AP203_E2_SINGLE_FILE, AP203_E2_SEPARATE_PART_FILES...
									const std::string	&in_ComponentID,
									bool				in_OnlyUpdateManifestForParts,
									bool				in_ChangeCaseOfPartStepFileToLowerCase,
									std::map<std::string, isis::CADComponentData> &in_CADComponentData_map )
												throw (isis::application_exception);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Description:
//		Create Creo model name with a unique suffix.  Examples:
//			Non-Family Table - Within in_AllowedSize:
//				in_UniqueNameIndex:							123
//				in_ModelName_CouldIncludeFamilyTableEntry:	bracket					// Length: 7
//				out_ModelName_Without_Suffix:               bracket					// Length: 7
//				out_ModelName_With_Suffix:                  bracketZ123Z			// Length: 12
//				out_CompleteName:                           bracketZ123Z			// Length: 12
//
//			Family Table  - Within in_AllowedSiZe
//				in_UniqueNameIndex:							123
//				in_ModelName_CouldIncludeFamilyTableEntry:	M3-50-10<NAS132-Bolt>		// Length: 21
//				out_ModelName_Without_Suffix:               NAS132-Bolt					// Length: 11
//				out_ModelName_With_Suffix:                  NAS132-BoltZ123Z			// Length: 16
//				out_CompleteName:                           M3-50-10<NAS132-BoltZ123Z>	// Length: 26
//
//			Non-Family Table - Truncation Required:	
//				in_UniqueNameIndex:                        99999
//				in_ModelName_CouldIncludeFamilyTableEntry: 12345678901234567890123456789   // Length: 29
//				out_ModelName_Without_Suffix:              12345678901234567890123456789   // Length: 29
//				out_ModelName_With_Suffix:                 123456789012345678901234Z99999Z //  Length: 31
//				out_CompleteName:                          123456789012345678901234Z99999Z //  Length: 31
//
//			Family Table - Truncation Required
//				in_UniqueNameIndex:                        123
//				in_ModelName_CouldIncludeFamilyTableEntry: Bolst_M3-50-10_Plated<NAS132>	// Length: 29
//				out_ModelName_Without_Suffix:              NAS132							// Length: 6
//				out_ModelName_With_Suffix:                 NASZ123Z							// Length: 8
//				out_CompleteName:                          Bolst_M3-50-10_Plated<NASZ123Z>  // Length: 31
//
//	Pre-Conditions:
//		in_ModelName_CouldIncludeFamilyTableEntry must be a legitimate Creo model name.
//
//		0 < in_UniqueNameIndex  < max unsigned int
//
//		if in_ModelName_CouldIncludeFamilyTableEntry describes a family table
//			then
//				in_ModelName_CouldIncludeFamilyTableEntry must contain '<', and '>', where '>' appears later in the string than '<'
//			Examples:
//				Engine						// Non family table example
//				Chassis_8_Wheel<Chassis>	// Family table example, If '<' exists then '>' must exist. One and only
//											// one '<' and one and only one '>'
// 	Post-Conditions	
//		throw application_exception if:
//			a) in_ModelName_CouldIncludeFamilyTableEntry.size() > 31
//			b) out_CompleteName.size() > 31. This would occur if the in_UniqueNameIndex forced the string to be too large.
//			c) in_ModelName_CouldIncludeFamilyTableEntry has a malformed name e.g. one "<" but not a closing ">"
//
//			Example of out_CompleteName.size() > 31
//				in_UniqueNameIndex:							12345
//				in_ModelName_CouldIncludeFamilyTableEntry:	Bolst_M3-50-10_Plated_S<NAS132>		// Length: 31
//				out_ModelName_Without_Suffix:               NAS132								// Length: 6
//				out_ModelName_With_Suffix:                  Z12345Z								// Length: 7
//				out_CompleteName:                           Bolst_M3-50-10_Plated_S<Z12345Z>	// Length: 32
//
//		if no exceptions, return out_... variables.  An example follows:	
//			a) Non Family Table - out_ModelName_With_Suffix="EngineZ1Z",	out_CompleteName="EngineZ1Z"
//			b) Family table		- out_ModelName_With_Suffix="ChassisZ1Z",	out_CompleteName="Chassis_8_Wheel<ChassisZ1Z>"


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void OrganizeMetricsBasedOnComponentIDs( 
							const std::list<CADComputation>							&in_Metrics,
							std::map<std::string, std::list<CADComputation>>	&out_componentID_to_ListofComputations_map,
							std::set<std::string>								&out_ComponentIDs_set );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ForEachLeafAssemblyInTheInputXML_AddInformationAboutSubordinates( 
					const std::vector<std::string>					&in_ListOfComponentIDsInTheAssembly, // This includes the assembly component ID
					int												&in_out_NonCyPhyID_counter,
					std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																		throw (isis::application_exception);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct ModelInstanceData
	{
		MultiFormatString	modelName;
		//ProMdlType			modelType;
		e_CADMdlType		modelType;
		ProSolid			modelHandle;

		// the assebled feature must be a PRO_PART, PRO_ASSEMBLY
		ProAsmcomp			assembledFeature;

		// Information from the top assembly to the instance
		ProSolid			topAssemblyModelHandle;
		std::list<int>			componentPaths;
		
	};

	// Description:
	//		Rename an instance in a Creo assembly to in_ToModelName.
	// Pre-Conditions:
	//		in_ToModelName must be a verbatim copy of the Creo model descirbed in in_FromModelInstanceData
	// Post-Condtions:
	//		if in_ToModelName is not a Creo model in the search path of the Creo session 
	//			then isis::application_exception) would be thrown.
	void  Assembly_RenameSubPartOrSubAssembly (	ModelInstanceData &in_FromModelInstanceData,  // Not constant because ProFeatureElemtreeExtract does not declare input only functions as constant
														MultiFormatString &in_ToModelName,
														ProMdl     &out_RenamedModelHandle)
																throw (isis::application_exception);


	

	// This function computes the bounding box based on excluding all geometry except for the solid geometry.
	// This means that datums, coordinate system, and sketch curves/lines would be excluded.
	// If the bounding box information has already been computed for in_ComponentInstanceID then those
	// values would be returned; otherwise, the values are computed and persisted in in_CADComponentData_map 
	// and then returned.
	void	RetrieveBoundingBox_ComputeFirstIfNotAlreadyComputed( 
								const std::string								&in_ComponentInstanceID,
								std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
								isis_CADCommon::Point_3D						&out_BoundingBox_Point_1,
								isis_CADCommon::Point_3D						&out_BoundingBox_Point_2,
								double							out_Dimensions_xyz[3] )
																		throw (isis::application_exception);	

	// e.g. "C:\\Users\\Public\\Documents\\META Documents\\MaterialLibrary\\MATERIALS_CREO_MTL"
	std::string CreoMaterialMTLFilesDir_Path();

	// e.g "C:\\Users\\Public\\Documents\\META Documents\\MaterialLibrary\\material_library.json"
	std::string MaterialsLibrary_PathAndFileName();


	void ValidatePathAndModelItem_ThrowExceptionIfInvalid( ProAsmcomppath	&in_Path, ProModelitem  &in_ModelItem )
		throw (isis::application_exception);

	void PopulateMap_with_Junctions_per_InputXMLConstraints( 
					cad::CadFactoryAbstract							&in_Factory,
					const std::vector<std::string>					&in_ListOfComponentIDsInTheAssembly, // This does not include the top-assembly component ID
					std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map,
					bool											in_Force = false)
																			throw (isis::application_exception);

	void	PopulateMap_with_JunctionDataInGlobalCoordinates( 
			const std::string								&in_AssemblyComponentID,
			const std::vector<std::string>					&in_ListOfComponentIDsInTheAssembly, // This does not include the top-assembly component ID
			std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																			throw (isis::application_exception);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Description:
	//		This function adds ComponentInstanceIDs to the 
	//		in_out_CADComponentData_map::�::constrainedTo_ComponentInstanceIDs_DerivedFromConstraintPairs, wherein the 
	//		ComponentInstanceIDs are either the part IDs that the datums contained in in_out_CADComponentData_map::�::constraintPairs
	//		reference, or the assembly IDs for the assemblies that are treated as one body (i.e. CADAssembly.xml 
	//		special instruction != HAS_KINEMATIC_JOINT ). Reference means the parts that contain the geometry 
	//		referenced by a datum.  See the contract for 
	//		FindPartsReferencedByFeature for more information about the meaning of a referenced part.
	//
	//	Pre-Conditions:
	//		in_TopAssemblyComponentInstanceID must be the top assembly for the entire assembly.  The feature IDs leading 
	//		to a sub-part/sub-assembly start at the top assembly.
	//
	//		in_AssemblyComponentIDs must be ComponentIDs in in_out_CADComponentData_map. 
	//
	//	Post-Conditions:
	//		if in_AssemblyComponentIDs.size() == 0, no action taken
	//
	//		if a referenced ComponentID is not found in in_FeatureIDs_to_ComponentInstanceID_hashtable
	//			then
	//				throw isis::application_exception
	void PopulateMap_with_ConstrainedToInfo_per_InputXMLConstraints ( 
			const std::string														&in_TopAssemblyComponentInstanceID,
			const std::vector<std::string>											&in_AssemblyComponentIDs,
			const std::unordered_map<IntList, std::string, ContainerHash<IntList>>	&in_FeatureIDs_to_ComponentInstanceID_hashtable,
			std::map<std::string, isis::CADComponentData>							&in_out_CADComponentData_map )
																		throw (isis::application_exception);

	void PopulateMap_with_Junctions_and_ConstrainedToInfo_per_CreoAsmFeatureTrees( 
			cad::CadFactoryAbstract													&in_Factory,
			const std::vector<std::string>											&in_AssemblyComponentIDs,
			const std::unordered_map<IntList, std::string, ContainerHash<IntList>>	&in_FeatureIDs_to_ComponentInstanceID_hashtable,
			std::map<std::string, isis::CADComponentData>							&in_out_CADComponentData_map )
																		throw (isis::application_exception);


	void ResolveAssemblyConstraints_AddMarkersToMap( 
			cad::CadFactoryAbstract													&in_Factory,
			const std::vector<std::string>											&in_AssemblyComponentIDs,
			std::unordered_map<IntList, std::string, ContainerHash<IntList>>		&in_FeatureIDs_to_ComponentInstanceID_hashtable,
			std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																		throw (isis::application_exception);
												
	void Populate_FeatureIDs_to_ComponentInstanceID_hashtable( 
					const std::vector<std::string>	&in_AssemblyComponentIDs,
					std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
					std::unordered_map<IntList, std::string, ContainerHash<IntList>> &out_FeatureIDs_to_ComponentInstanceID_hashtable );

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// For valid joint information:
	//		1.  All parts that are not within a treat-as-one-body assembly (i.e. CADAssembly.xml 
	//			special instruction != HAS_KINEMATIC_JOINT ) must be connected to at least one other part.
	//		2.  For a kinematic joint between two parts or a part and assembly, only the two parts or part and assembly can be 
	//			engaged in the constraints.
	//			In other words, if part A is constrained to part B via a kinematic joint (i.e. revolute, prismatic...),
	//			then the constraint features must be between part A and part B.  They can't be between a part other than
	//			A and B.  Another valid example is part A is constrained to datums in assembly B wherein assembly B is a
	//			treat-as-one-body assembly. Only A and B can be engaged in the kinematic joint.
	//			For the assembly example, the assembly B could have datums that reference more than one part that are
	//			subordinate to B and this would still be acceptable since A is being constrained to B; howerver, the
	//          more-than-one-part condition may not be allowed via the Creo UI, but appears to be acceptable via the SDK.
	//			The Creo UI enforces this for kinematic joints.  We must ensure that CADAssembly.xml does not
	//			violate this rule.
	//			Note -	A kinematic joint between two parts is at the set level.  This means that set 0 could be a 
	//					a kinematic joint between one end of bar A to another bar B, and set 1 could be a kinematic
	//					joint from the other end of bar A to a another bar C.  
	//
	//	This function populates out_Errors with error descriptions.  
	//	If (out_Errors.size() == 0 )
	//		then
	//			no errors were found.
	//
	//	This function only checks parts.  If in_ListOfComponentIDsInTheAssembly contains assemblies, then the assemblies are skipped.	
	void CheckValidityOfJointInformation( 
			const std::vector<std::string>					&in_ListOfComponentIDsInTheAssembly, 
			std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
			std::vector<std::string>						&out_Errors );


	//	Description:
	//		Returns if in_ComponentInstanceID is a CyPhy leaf assembly.  This is  determineed  by checking if 
	//		the immediate (i.e. not grandchildren) children of in_ComponentInstanceID have the setting of 
	//		INITIAL_SOURCE_DERIVED_FROM_LEAF_ASSEMBLY_DESCENDANTS
	//
	//	Pre-Conditions:
	//		ForEachLeafAssemblyInTheInputXML_AddInformationAboutSubordinates must have been invoked before 
	//		calling this function; and if not, CyPhyLeafAssembly will always return false.
	//
	//	Post-Conditions:
	//		if in_ComponentInstanceID is a part
	//			return false
	//		if the first immediate child has the setting of INITIAL_SOURCE_DERIVED_FROM_LEAF_ASSEMBLY_DESCENDANTS
	//			return true
	//		else
	//			return false
	bool CyPhyLeafAssembly (	const std::string								&in_ComponentInstanceID,
								std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map );


	void AddBoundingBoxValuesToMap( 
						const std::vector<std::string>					&in_AssemblyComponentIDs,
						std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map) throw (isis::application_exception);

	void AddMassPropertyValuesToMap( 
						const std::vector<std::string>					&in_AssemblyComponentIDs,
						std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map) 
																				throw (isis::application_exception);

}

#endif