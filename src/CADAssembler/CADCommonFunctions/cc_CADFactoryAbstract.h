
#ifndef CC_CAD_FACTORY_ABSTRACT_H
#define CC_CAD_FACTORY_ABSTRACT_H

#include <string>
#include "cc_Joint.h"
#include "cc_CommonStructures.h"
#include "isis_application_exception.h"
#include "cc_CreateAssemblyInputArgumentsParser.h"
#include "cc_MetaLinkInputArgumentsParser.h"
#include "cc_ExtractACMInputArgumentsParser.h"
//#include "cc_AssemblyUtilities.h"
#include <boost/smart_ptr.hpp>
#include <unordered_map>

/**
This abstract class provides factories for the
concrete CAD systems.
*/

namespace isis {
namespace cad {


//enum e_OpenMETAApplication
//{
//	OPENMETA_CREATE_ASSEMBLY,
//	OPENMETA_META_LINK,
//	OPENMETA_EXTRACT_ACM
//};

class IAssembler {
public:
	// provide the name of the concrete assembler
	virtual std::string name() = 0;

	// virtual std::vector<Joint> get_joints() = 0;
	
	/**
	Given a set of constraints infer the joint.
	*/
	virtual std::vector<Joint::pair_t>  extract_joint_pair_vector
		(const std::string in_component_instance_id,
		 std::vector<ConstraintPair> in_component_pair_vector,
		 std::map<std::string, isis::CADComponentData> &	in_CADComponentData_map)
		 = 0;

	/**
	There is an CAD specific object for the assembly.
	This function sets that object.
	*/
	virtual void* get_assembly_component
		( const std::string in_working_directory,
		  const std::string &	in_id, 
		  std::map<std::string, isis::CADComponentData> &	in_map)
		= 0;


	// This function does any setup that is necessary before invoking the CAD application
	//virtual void SetupCADEnvirnoment ( const DataContainer &in_DataContainer) throw (isis::application_exception) = 0;

};


class ICADSession {
protected:
	MultiFormatString cADWorkingDirectory;
public:
	// provide the name of the concrete assembler
	virtual std::string name() = 0;

	virtual void startCADProgram( const std::string &in_StartCommand ) const throw (isis::application_exception) = 0;
	virtual void stopCADProgram( )  const throw (isis::application_exception) = 0;

	// The CAD session must have been started before calling this function.  This function retrieves the version
	// number of the current CAD session.
	virtual void getCADProgramVersion(	bool		&out_IntVersionNumber_Set,  
										int		&out_IntVersionNumber, 
										bool		&out_FloatVersionNumber_Set,
										double  &out_FloatVersionNumber )  const throw (isis::application_exception) = 0;

	// The working directory is passed in as a command line argument to the CreateAssemble... exe.
	// cc (CAD Common) functions automatically call setCADWorkingDirectory.  setCADWorkingDirectory must persist the command line 
	// argument in ICADSession::cADWorkingDirectory.  Failure to persist this value will cause an exception when getCADWorkingDirectory
	// is invoked.
	//
	// For Creo setCADWorkingDirectory does:
	//	1)	saves in_MultiFormatString to ICADSession::cADWorkingDirectory.  This is so other functions can retrieve it via
	//		getCADWorkingDirectory
	//	2)  call the Creo toolkit API to set the working directory.  This is because Creo has the concepty of a working 
	//		directroy which must be set.  Models (parts, assemblies, STEP files...) are save to this working directory. 
	//
	// Other CAD systems may not have the concept of setting a working directory.  Other systems may simply prompt
	// or allow the user to specify where to  save the models.  If that is the case, then setCADWorkingDirectory should
	// only set ICADSession::cADWorkingDirectory.  Use getCADWorkingDirectory to determine where to save the model.
	virtual void setCADWorkingDirectory ( const MultiFormatString &in_MultiFormatString ) throw (isis::application_exception) = 0;

	// There is NO NEED to OVERRIDE this function.  There is a default implementation that returns ICADSession::cADWorkingDirectory.
	// ICADSession::cADWorkingDirectory must be set by setCADWorkingDirectory.  If this function is called prior to calling
	// setCADWorkingDirectory then application_exception will be thown.
	virtual MultiFormatString getCADWorkingDirectory () throw (isis::application_exception);


	// Description:
	//		The setupCADEnvironment functions set up the environment that is necessary to run the CAD application.
	//		After running this command, the API call to run the CAD application should work.
	//		For Creo this command is isis::isis_ProEngineerStart(...).
	//
	//		This program also writes special files (e.g. configuration files) 
	//		that are needed for the CAD program to run.
	//
	// Pre-Conditions:
	//		None
	//
	// Post-Conditions:
	//		isis::application_exception will be thrown if
	//			1.	the supported CAD application is not found. Typically, this program only supports one version of 
	//				the CAD application (e.g. 3.0 but not 2.0 or 1.0).
	//			2.  the registry entry META_PATH does not exist. This should typically be in the registry
	//				at HKEY_LOCAL_MACHINE\SOFTWARE\META.  The META installer creates this registry entry.
	//		
	//		If no exceptions, then the CAD environment would be initialized and the out_* variables returned.
	//

	virtual void setupCADEnvironment(	

			const CreateAssemblyInputArguments &in_CreateAssemblyInputArguments,

			// This is the command or other information that would be passed to the API call that starts the CAD application
			// e.g. passed to isis::isis_ProEngineerStart(...)
			std::string		&out_CADStartCommand,	

			// This directory contains the executables, starter CAD models, schemas, and 
			// templates that are necessary for the CreateAssembly and MetaLink programs to run.
			// e.g. C:\Program Files (x86)\META\bin\CAD\Creo
			std::string		&out_CADExtensionsDir,

			// This is the CAD model name of the empty starter assembly (e.g. zzz_template_assy_mmks_creo.asm )
			// When this program creates a CAD assembly, it copies this assembly to the working directory,
			// renames it to the correct name, and adds parts/sub-assemblies.
			std::string		&out_TemplateFile_PathAndFileName ) const throw (isis::application_exception) = 0;

	virtual void setupCADEnvironment(	

			const MetaLinkInputArguments &in_MetaLinkInputArguments,

			// This is the command or other information that would be passed to the API call that starts the CAD application
			// e.g. passed to isis::isis_ProEngineerStart(...)
			std::string		&out_CADStartCommand,	

			// This directory contains the executables, starter CAD models, schemas, and 
			// templates that are necessary for the CreateAssembly and MetaLink programs to run.
			// e.g. C:\Program Files (x86)\META\bin\CAD\Creo
			std::string		&out_CADExtensionsDir,

			// This is the CAD model name of the empty starter assembly (e.g. zzz_template_assy_mmks_creo.asm )
			// When this program creates a CAD assembly, it copies this assembly to the working directory,
			// renames it to the correct name, and adds parts/sub-assemblies.
			std::string		&out_TemplateFile_PathAndFileName ) const throw (isis::application_exception) = 0;


	virtual void setupCADEnvironment(	

			const ExtractACMInputArguments &in_ExtractACMInputArguments,

			// This is the command or other information that would be passed to the API call that starts the CAD application
			// e.g. passed to isis::isis_ProEngineerStart(...)
			std::string		&out_CADStartCommand,	

			// This directory contains the executables, starter CAD models, schemas, and 
			// templates that are necessary for the CreateAssembly and MetaLink programs to run.
			// e.g. C:\Program Files (x86)\META\bin\CAD\Creo
			std::string		&out_CADExtensionsDir,

			// This is the CAD model name of the empty starter assembly (e.g. zzz_template_assy_mmks_creo.asm )
			// When this program creates a CAD assembly, it copies this assembly to the working directory,
			// renames it to the correct name, and adds parts/sub-assemblies.
			std::string		&out_TemplateFile_PathAndFileName ) const throw (isis::application_exception) = 0;

};


class IModelHandling {
public:
	// provide the name of the concrete assembler
	virtual std::string name() = 0;

	//	out_RetrievedModelHandle handle is an in memory handle to the model
	//	This function is useful for the case where a simplified represenations is not needed. 
	//	For example, reading a model (from the component library) and saving it to the current 
	//  working directory.  This example is equivalent to copying a model.
	//	This function is also used for unassembled components, which do not appear in in_CADComponentData_map
	virtual void cADModelRetrieve(	const isis::MultiFormatString		&in_ModelName,		// model name without the suffix.  e.g  1234567  not 1234567.prt
									e_CADMdlType 						in_ModelType,		// CAD_MDL_ASSEMBLY CAD_MDL_PART,
									void 								**out_RetrievedModelHandle_ptr ) const throw (isis::application_exception) = 0;

	// Use this function when retrieving a model that might have a simplified representation specified.
	// We cannot just specidfy in_ComponentInstanceID and in_CADComponentData_map because unassembled 
	// components would not be included in in_CADComponentData_map.  Unassembled components are specified
	// in CADAssembly.xml and are not placed in in_CADComponentData_map because most of the information
	// in in_CADComponentData_map is not relevant to unassembled components.
	virtual void cADModelRetrieve(	const std::string					&in_ComponentInstanceID,		// this is included for logging purposes; otherwise this function does not use in_ComponentInstanceID	
									const isis::MultiFormatString		&in_ModelName,				// model name without the suffix.  e.g  1234567  not 1234567.prt
									e_CADMdlType 						in_ModelType,				// CAD_MDL_ASSEMBLY or CAD_MDL_PART
									const MultiFormatString				&in_GeometryRepresentation, // This would be null if no simplified representation was specified
									void 								**out_RetrievedModelHandle_ptr ) const throw (isis::application_exception) = 0;


	// in_ModelHandle handle is an in memory handle to the model
	virtual void cADModelSave(	   void 								*in_ModelHandle ) const throw (isis::application_exception) = 0;

	virtual void cADModelFileCopy (e_CADMdlType 						in_ModelType,
								   const isis::MultiFormatString		&in_FromModelName,
								   const isis::MultiFormatString        &in_ToModelName) const throw (isis::application_exception) = 0;

	virtual void cADModelSave( 
					const std::string								&in_ComponentInstanceID,
					std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map )
																		throw (isis::application_exception) = 0;	

	virtual void extractModelNameAndFamilyTableEntry(	
		// This could be either:
		//		a) CAD model name (e.g. wheel_bearing or 123456)
		//		b) CAD model name with a family table entry 
		//		   (e.g. for Creo Chassis_8_Wheel<Chassis>, where Chassis is the model name and Chassis_8_Wheel is the family table entry)
		const std::string	&in_OrigName, 
		// Model name (e.g. wheel_bearing, 123456, Chassis)
		std::string			&out_ModelName,
		// FamilyTableEntry (e.g. "", Chassis_8_Wheel), Note - "" indicated no family table entry.
		std::string			&out_FamilyTableEntry,
		// FamilyTableModel True/False, False is no familty table entry
		bool				&out_FamilyTableModel ) const throw (isis::application_exception) = 0;

		// e.g. in Creo 
		// in_ModelName = Chassis
		// in_FamilyTableEntry = Chassis_8_Wheel
		// return Creo Chassis_8_Wheel<Chassis>
		// Note - Actual entries are CAD system dependent
		virtual std::string buildAFamilyTableCompleteModelName ( const std::string &in_ModelName,
																 const std::string &in_FamilyTableEntry ) = 0;

		virtual std::string combineCADModelNameAndSuffix ( const std::string &in_ModelName, e_CADMdlType in_ModelType )
															throw (isis::application_exception) = 0 ;

};


// Forward declare, because used in CadFactoryAbstract 
class CadFactoryAbstract;


class IModelOperations {
public:
	// provide the name of the concrete assembler
	virtual std::string name() = 0;

	//	The CADAssembly.xml contains an assembly tree (i.e. hierarchy), where leafs in the 
	//	tree can be parts or sub-assemblies.  For the case where where the leaf is an assembly, 
	//	this function completes the hierarchy in in_out_CADComponentData_map such that the tree 
	//	is complete.  The in_out_NonCyPhyID_counter is a counter that is used to create component
	//	IDs for the subordinate parts/sub-assemblies. 
	virtual void forEachLeafAssemblyInTheInputXML_AddInformationAboutSubordinates( 
					const std::vector<std::string>					&in_ListOfComponentInstanceIDsInTheAssembly, // This includes the assembly component ID
					int												&in_out_NonCyPhyID_counter,
					std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																		throw (isis::application_exception) = 0;

	//	A CAD model hierarchy is a tree structure wherein an assembly points to sub-assemblies and/or parts.
	//	This function will replace one assembly or one part as defined 
	//		in_CopyModelDefinition.fromModelName
	//  	in_CopyModelDefinition.toModelName
	//	The replacement process requires two steps:
	//		1) Update the CAD assembly hiearchy (i.e. update the CAD application (e.g. Creo) data structures)
	//		2) Update the in_out_CADComponentData_map
	//	Before this function is invoked
	//		1) The in_CopyModelDefinition.toModelName must exist in the CAD search path.
	//		2) The assembly containing in_CopyModelDefinition.fromModelName must be in memory
	//		3) In the case of Creo, in_CopyModelDefinition.toModelName must be in memory
	virtual void modify_CADInternalHierarchyRepresentation_CADComponentData__ForCopiedModel( 
					const std::string								&in_TopAssemblyComponentID,
					const isis::CopyModelDefinition					&in_CopyModelDefinition, 
					std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																		throw (isis::application_exception) = 0;			

	// This function should probably be further divided so that the following function is general (i.e. cad common (cc_)).
	// Calls to the CADFactroy would be made from the general function.
	// ?? Documenation of this function is needed.
	virtual void populateMap_with_Junctions_and_ConstrainedToInfo_per_CADAsmFeatureTrees( 
			//cad::CadFactoryAbstract													&in_Factory,
			const std::vector<std::string>											&in_AssemblyComponentInstanceIDs,
			const std::unordered_map<IntList, std::string, ContainerHash<IntList>>	&in_FeatureIDs_to_ComponentInstanceID_hashtable,
			std::map<std::string, isis::CADComponentData>	&in_out_CADComponentData_map )
																		throw (isis::application_exception) = 0;

	virtual void retrieveTranformationMatrix_Assembly_to_Child (  
								const std::string									&in_AssemblyComponentInstanceID,
								const std::string									&in_ChildComponentID,
								std::map<std::string, isis::CADComponentData>		&in_CADComponentData_map,  
								bool  in_bottom_up,
								double out_TransformationMatrix[4][4] )  throw (isis::application_exception) = 0;

	virtual void retrieveTranformationMatrix_Assembly_to_Child (  
								const std::string									&in_AssemblyComponentInstanceID,
								const std::list<int>									&in_ChildComponentPaths,
								std::map<std::string, isis::CADComponentData>		&in_CADComponentData_map,  
								bool  in_bottom_up,
								double out_TransformationMatrix[4][4] )  throw (isis::application_exception) = 0;


	// This function computes the bounding box based on excluding all geometry except for the solid geometry.
	// This means that datums, coordinate system, and sketch curves/lines would be excluded.
	// If the bounding box information has already been computed for in_ComponentInstanceID then those
	// values would be returned; otherwise, the values are computed and persisted in in_CADComponentData_map 
	// and then returned.
	virtual void	 retrieveBoundingBox_ComputeFirstIfNotAlreadyComputed( 
								const std::string								&in_ComponentInstanceID,
								std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
								isis_CADCommon::Point_3D							&out_BoundingBox_Point_1,
								isis_CADCommon::Point_3D							&out_BoundingBox_Point_2,
								double											out_Dimensions_xyz[3] )
																		throw (isis::application_exception) = 0;


	// The point coordinates are relative to the coordinate system in the in_AssemblyComponentInstanceID assembly
	virtual void retrievePointCoordinates(	const std::string								&in_AssemblyComponentInstanceID,
											const std::string								&in_PartComponentID,
											std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
											const MultiFormatString							&in_PointName,
											CADPoint											&out_CADPoint) 
																				throw (isis::application_exception)	= 0;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Description: 
	//		This function resolves the references of in_FeatureName to the parts that are referenced by in_FeatureName.  
	//		For example, in_FeatureName could be a datum (i.e. plane, axis, point, or coordinate system) and that datum 
	//		could be defined by pointing to part geometry.  This function follows those refereces until it finds the parts 
	//		that own the referenced geometry.  A common use case is a datum plane in an assembly where the datum plane 
	//		points to a second datum plane in a sub-part, wherein the second datum plane points to geometry in the sub-part.  
	//		This function would return the ComponentInstanceIDs of the parts that own the referenced geometry.
	//
	//		It should be noted that a datum can reference more than one geometries and those geometries can be in separate
	//		parts.
	// 
	//	Pre-Conditions:
	//		This function should be called with out_ComponentInstanceIDs_of_PartsReferencedByFeature_set being empty.
	//
	//		in_TopAssemblyModel_ComponentInstanceID must be in in_CADComponentData_map and be the top assembly for the 
	//		entire assembly structure containing in_ComponentInstanceID
	//
	//		in_ComponentInstanceID must be in CADComponentData_map
	//
	//	Post-Conditions
	//		if in_FeatureName not found in in_ComponentInstanceID then throw throw (isis::application_exception)
	//
	//		If no exceptions 
	//			then 
	//				out_ComponentInstanceIDs_of_PartsReferencedByFeature_set would be populated with 0 to many ComponentInstanceIDs.
	//				It could be 0 because in_FeatureName might only have references to an assembly and this function only finds 
	//				references to parts.

	virtual void findPartsReferencedByFeature(	
						const std::string								&in_TopAssemblyComponentID, 
						const std::string								&in_ComponentInstanceID,
						const MultiFormatString							&in_FeatureName,
						e_CADFeatureGeometryType							in_FeatureGeometryType,
						const std::unordered_map<IntList, std::string, ContainerHash<IntList>>		&in_FeatureIDs_to_ComponentInstanceID_hashtable,
						std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
						std::set<std::string>							&out_ComponentInstanceIDs_of_PartsReferencedByFeature_set)
																			throw (isis::application_exception) = 0;


	// See the ModelOperationsCreo::retrieveMassProperties funtion for how out_MassProperties should be set
	virtual void retrieveMassProperties( 
						const std::string								&in_ComponentInstanceID,
						std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,
						MassProperties									&out_MassProperties) 
																				throw (isis::application_exception) = 0;


	virtual void  convertCADUnitToGMEUnit_Distance( const MultiFormatString &in_DistanceUnit, std::string &out_ShortName, std::string &out_LongName  )
																											throw (isis::application_exception) = 0;

	virtual void  convertCADUnitToGMEUnit_Mass( const MultiFormatString &in_MassUnit,  std::string &out_ShortName, std::string &out_LongName  )
																											throw (isis::application_exception) = 0;

	virtual void convertCADUnitToGMEUnit_Force ( const MultiFormatString &in_ForceUnit, std::string &out_ShortName, std::string &out_LongName  )
																											throw (isis::application_exception) = 0;

	virtual void convertCADUnitToGMEUnit_Time ( const MultiFormatString &in_TimeUnit, std::string &out_ShortName, std::string &out_LongName  )
																											throw (isis::application_exception) = 0;
	virtual void convertCADUnitToGMEUnit_Temperature ( const MultiFormatString &in_TemperatureUnit, std::string &out_ShortName, std::string &out_LongName  )
																											throw (isis::application_exception) = 0;

	virtual void retrieveCADModelUnits( 
					const std::string								&in_ComponentInstanceID,
					std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map,  
					CADModelUnits									&out_CADModelUnits )
																	throw (isis::application_exception) = 0;

	// Only parts have designated materials.  Assemblies do not have designated materials; and thus, an exception will be
	// thrown if this functions is called if in_ComponentInstanceID is a assembly.
	// A part can have no material defined. For that case this function will throw an exception.
	virtual MultiFormatString retrieveMaterialName( 	const std::string								&in_ComponentInstanceID,
													std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map) 
																											throw (isis::application_exception) = 0;

	// This function only adds in_ModelComponentIDsToAdd parts/sub-assemblies to the in_AssemblyComponentInstanceID assembly.  It does
	// not constrain the added parts/sub-assemblies.
	virtual void addModelsToAssembly( 
					const std::string									&in_AssemblyComponentInstanceID,
					const std::list<std::string>							&in_ModelComponentIDsToAdd,
					std::map<std::string, isis::CADComponentData>		&in_CADComponentData_map,
					int													&in_out_AddedToAssemblyOrdinal)
																			throw (isis::application_exception) = 0;

	// Description:
	// -----------
	//	This function constrains a single CAD model.  Constraining a CAD model entails aligning/mating features 
	//	in the assembled CAD model to features in models that are already in the assembly or features in the top assembly.  
	//	The features would be planes, axes, points, etc.
	//
	//	Implementing this function in Creo was very difficult.  This is because in Creo you must:
	//		a)	tell the systems what type of joint is being created.  For example, an axis and and an orthogonal plane 
	//			would normally define a revolute joint, but you must tell Creo that it is a revolute joint.  The system does
	//			not figure this out on its own. To know the type joint, there must be code that determines that the plane is 
	//			orthogonal to the axis.
	//		b)	order the constraints in a manner compatible to joint type.  For example, for a revolute joint, the axis 
	//			must be constrained first; otherwise, Creo will not treat the joint as a revolute joint.
	//		c)	properly orient axes.  In Creo, an axis has an orientation, which is not directly exposed via the UI; however, 
	//			from the toolkit (i.e. API) perspective the orientation must be correct to result in solvent kinematic joint.
	//		d)	treat the added model (e.g. part/sub-assembly) and the constrained to model(s) differently in the API.  The Creo 
	//			API makes a distinction of which model is the added model and thus this function must determine that.  This is 
	//			not as straightforward as one might think.  For example, the added model could be an assembly and the features used 
	//			to constrain the added assembly could be models (parts/sub-assemblies) subordinate to the added assembly. 
	//
	//	Ideally, a list of constraints could be given to the system and the system would order them properly and determine the 
	//	correct type of joint (e.g. fixed, revolute, cylindrical, planar, slider...).  Hopefully, other CAD systems will do 
	//	this and the complexity of the Creo implementation for this function will not be needed for other CAD systems.  If the
	//  complexity is needed for other CAD systems, then the Creo implementation of this function should be refactored so that
	//	most of the functionality goes through the abstract factory.
	//
	//	In general this function performs the following steps:
	//	if guide is present (i.e. in CADAssembly.xml):
	//		a)	apply the constraints including the guide.  This provides an orientation of the part.  For example, if the joint 
	//			is a revolute joint, then the guide would provide the clocking position.
	//		b)	determine the transformation to the part in space.
	//		c)	remove the constraints
	//		d)	apply the transformation
	//		d)	apply the constraints without the guide
	//
	//	if guide is not present:
	//		a) apply the constraints
	//		
	//	if InputJoint information (e.g. limits of rotation) is present (i.e. in CADAssembly.xml):
	//		a) apply the joint information in addition to the previously applied constraints
	//
	//	Regenerate - Normally would not need to regenerate the entire assembly, just the constrained model.
	//
	// Pre-Conditions
	// --------------
	//	1) in_AssemblyComponentInstanceID and in_ComponentIDToBeConstrained must be in memory.  
	//
	// Post-Conditions
	// ---------------
	//  if no exceptions:
	//		1) in_ComponentIDToBeConstrained would be properly constrained
	//		2) if constraint features would result in a kinematic constraint, then the joint will be kinematic joint
	//		3) if limits (e.g. rotation limits) exist, then the joint will adhere to those limits.
	//
	//	an exception would occur if:
	//		1)	if the constraint features are not compatible or possible.  For example, if one model has three 
	//			orthogonal planes that are being constrained to another model but the planes are not orthogonal
	//			in the other model, then the features are not compatible.
	//		2)	if the model will not regenerate after applying the constraints.  This would indicate that the
	//			the constraints are malformed.
	virtual bool applySingleModelConstraints( 
				const std::string								&in_AssemblyComponentInstanceID,
				const std::string								&in_ComponentIDToBeConstrained,		
				std::map<std::string, isis::CADComponentData>	&in_CADComponentData_map )
																			throw (isis::application_exception) = 0;


	//	If a particular CAD system does not support a particular output format (e.g. STEP AP203_E2) then:
	//		1) this program (i.e. the overall program) does not return an error.  
	//		2) the particular output format will not be created.
	//		3) a warning message will be logged.
	//		4) in the directory that would normally contain the exported data-exchange files, _FAILED_Data_Exchange.txt will be 
	//			created and will explain that the particular CAD system does not support that export type.
	//	In general, it is the responsibility of the CyPhy Test Bench developer to assure the format that is 
	//	requested is supported by the particular CAD system.
	virtual bool	 dataExchangeFormatSupported(const DataExchangeSpecification &in_DataExchangeSpecificationt) = 0;  

	//	Notes:
	//		1)	We export files (e.g. unassembled components) that are not in in_CADComponentData_map.  Therefore, we
	//			must use the model handle. 
	//		2)  in_ModelHandle_ptr must point to a model in memory.  cc_DataExchange.c/.h retrieves a model before 
	//			calling this function.
	//		3)  The directory defined by in_OutputDirectoryPath must exist prior to calling this function. cc_DataExchange.c/.h 
	//			assures that this directory exists.
	//		4)	This function may change the working directory to in_OutputDirectory. Typically, this would be to 
	//			a sub-directory (.e.g. STEP) under the current working directory.  It is the caller's responsibility to change the
	//			the working directory back to the desired location after calling this function. cc_DataExchange.c/.h performs
	//			this change back to the original location automatically. 
	//		5)	If a format (i.e. format specified in in_DataExchangeSpecification) is not supported by the particular CAD
	//			system, then the lack of support should be logged and a filed file created, but an exception should not be thrown.  
	//			This is necessary because all formats will not be supported by all CAD systems.  The Test Bench developer will need to be 
	//			cognizant of what formats are supported by a particular CAD system.
	//			The failed file should be created in in_OutputDirectoryPath and should be called _FAILED_Data_Exchange.txt.  This file 
	//			should contain a message indicating the the particular format is not supported.
	virtual void exportDataExchangeFile_STEP(				void 							*in_ModelHandle_ptr,
															e_CADMdlType						in_ModelType,
															const DataExchangeSpecification	&in_DataExchangeSpecification,
															const MultiFormatString			&in_OutputDirectoryPath,		// Only the path to the directory
															const MultiFormatString			&in_OutputFileName)		    // This the complete file name (e.g. bracket_asm.stp)
																							throw (isis::application_exception) = 0;

	// Notes - Same notes as exportDataExchangeFile_STEP
	virtual void exportDataExchangeFile_Stereolithography(	void 							*in_ModelHandle_ptr,
															e_CADMdlType						in_ModelType,
															const DataExchangeSpecification	&in_DataExchangeSpecification,
															const MultiFormatString			&in_OutputDirectoryPath,		// Only the path to the directory
															const MultiFormatString			&in_OutputFileName)		    // This the complete file name (e.g. bracket_asm.stp)
																							throw (isis::application_exception) = 0;

	// Notes - Same notes as exportDataExchangeFile_STEP
	virtual void exportDataExchangeFile_Inventor(			void 							*in_ModelHandle_ptr,
															e_CADMdlType						in_ModelType,
															const DataExchangeSpecification	&in_DataExchangeSpecification,
															const MultiFormatString			&in_OutputDirectoryPath,		// Only the path to the directory
															const MultiFormatString			&in_OutputFileName)		    // This the complete file name (e.g. bracket_asm.stp)
																							throw (isis::application_exception) = 0;


	// Notes - Same notes as exportDataExchangeFile_STEP
	virtual void exportDataExchangeFile_DXF(					void 							*in_ModelHandle_ptr,
															e_CADMdlType						in_ModelType,
															const DataExchangeSpecification	&in_DataExchangeSpecification,
															const MultiFormatString			&in_OutputDirectoryPath,		// Only the path to the directory
															const MultiFormatString			&in_OutputFileName)		    // This the complete file name (e.g. bracket_asm.stp)
																							throw (isis::application_exception) = 0;

	// Notes - Same notes as exportDataExchangeFile_STEP
	virtual void exportDataExchangeFile_Parasolid(			void 							*in_ModelHandle_ptr,
															e_CADMdlType						in_ModelType,
															const DataExchangeSpecification	&in_DataExchangeSpecification,
															const MultiFormatString			&in_OutputDirectoryPath,		// Only the path to the directory
															const MultiFormatString			&in_OutputFileName)		    // This the complete file name (e.g. bracket_asm.stp)
																							throw (isis::application_exception) = 0;




};




class CadFactoryAbstract {
public:
	typedef boost::shared_ptr<CadFactoryAbstract> ptr;

	// provide the name of the concrete factory
	virtual std::string name() = 0;

	/**
	Get the CAD specific concrete functor for
	manipulating the assembly.
	*/
	virtual IAssembler&					 get_assembler() = 0;
	virtual ICADSession&				     getCADSession() = 0;
	virtual IModelHandling&				 getModelHandling() = 0;
    virtual IModelOperations&            getModelOperations() = 0;
};



class CadFactoryAbstract_global
{
	private:

		static CadFactoryAbstract_global *s_instance;

		CadFactoryAbstract::ptr cADFactory_ptr;

		bool factorySet;

	public:

		CadFactoryAbstract_global(): factorySet(false) {}

		static CadFactoryAbstract_global *instance();

		// Should call this function one and only one time.  Calling a second time will result in an exception.
		void setCadFactoryAbstract_ptr(isis::cad::CadFactoryAbstract::ptr in_cADFactory_ptr ) throw (isis::application_exception);

		// setCadFactoryAbstract_ptr must be called before calling this function; otherwise and exception would be thrown.
		CadFactoryAbstract::ptr getCadFactoryAbstract_ptr() throw (isis::application_exception);
};



} // cad
} // isis

#endif // CAD_FACTORY_ABSTRACT_H