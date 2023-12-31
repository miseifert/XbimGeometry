
#include <TopExp.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GC_MakeEllipse.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeLine.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgo_Loop.hxx>
#include <Geom_OffsetCurve.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom_Curve.hxx>
#include <BRep_Tool.hxx>
#include <BRepPrim_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <ShapeFix_Face.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TShort_Array1OfShortReal.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <ShapeFix_Solid.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepGProp_Face.hxx>

#include <GeomProjLib.hxx>
#include <BRepOffsetAPI_NormalProjection.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeFix_Edge.hxx>
#include <Geom_Ellipse.hxx>
#include <gp_Ax1.hxx>
#include <Geom_BSplineSurface.hxx>

#include <CSLib_NormalStatus.hxx>
#include <CSLib.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimWireSet.h"
#include "XbimCompound.h"
#include "XbimSolidSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimPoint3DWithTolerance.h"

#include "XbimGeometryCreator.h"
#include "XbimConvert.h"
#include "XbimOccWriter.h"
#include "Factories/Unmanaged/NCurveFactory.h"

#include "./Factories/SolidFactory.h"
#include "Factories/BooleanFactory.h"

using namespace System::Linq;
using namespace Xbim::Common;
using namespace Xbim::Geometry::Services;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimSolid::InstanceCleanup()
		{
			System::IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, System::IntPtr::Zero);
			if (temp != System::IntPtr::Zero)
				delete (TopoDS_Solid*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

#pragma region Constructors

		XbimSolid::XbimSolid(const TopoDS_Solid& solid, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			pSolid = new TopoDS_Solid();
			*pSolid = solid;
		}

		XbimSolid::XbimSolid(const TopoDS_Solid& solid, Object^ tag, ModelGeometryService^ modelService) : XbimSolid(solid, modelService)
		{
			Tag = tag;
		}


		XbimSolid::XbimSolid(IIfcSolidModel^ solid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcManifoldSolidBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcSweptAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, overrideProfileDef, logger);
		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolid^ solid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolidTapered^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, nullptr, logger);

		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, overrideProfileDef, logger);

		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, overrideProfileDef, logger);

		}
		XbimSolid::XbimSolid(IIfcExtrudedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, overrideProfileDef, logger);

		}
		

		XbimSolid::XbimSolid(IIfcExtrudedAreaSolidTapered^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, nullptr, logger);

		}
		
		XbimSolid::XbimSolid(IIfcSweptDiskSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}



		XbimSolid::XbimSolid(IIfcSectionedSpine^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcBoundingBox^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}



		XbimSolid::XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, nullptr, logger);
		}

		

		XbimSolid::XbimSolid(IIfcHalfSpaceSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}


		XbimSolid::XbimSolid(XbimRect3D rect3D, double tolerance, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(rect3D, tolerance, logger);
		}

		XbimSolid::XbimSolid(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(IIfcSolid, logger);
		}

		XbimSolid::XbimSolid(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(solid, logger);
		}



		XbimSolid::XbimSolid(IIfcCsgPrimitive3D^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}



		XbimSolid::XbimSolid(IIfcSphere^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcBlock^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRightCircularCylinder^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRightCircularCone^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRectangularPyramid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcFixedReferenceSweptAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, nullptr, logger);
		}

		XbimSolid::XbimSolid(IIfcFixedReferenceSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(repItem, overrideProfileDef, logger);
		}
#pragma   endregion


#pragma region Equality Overrides

		bool XbimSolid::Equals(Object^ obj)
		{
			// Check for null
			if (obj == nullptr) return false;

			// Check for type
			if (this->GetType() != obj->GetType()) return false;

			// Cast as XbimVertex
			XbimSolid^ s = (XbimSolid^)obj;
			return this == s;
		}

		bool XbimSolid::Equals(IXbimSolid^ obj)
		{
			// Check for null
			if (obj == nullptr) return false;
			return this == (XbimSolid^)obj;
		}

		int XbimSolid::GetHashCode()
		{
			if (!IsValid) return 0;
			return pSolid->HashCode(System::Int32::MaxValue);
		}

		bool XbimSolid::operator ==(XbimSolid^ left, XbimSolid^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Solid&)left).IsEqual(right) == Standard_True;

		}

		bool XbimSolid::operator !=(XbimSolid^ left, XbimSolid^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region Initialisation methods

		void XbimSolid::Init(IIfcSolidModel^ solid, ILogger^ logger)
		{
			IIfcSweptAreaSolid^ extrudeArea = dynamic_cast<IIfcSweptAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea, nullptr, logger);
			IIfcSweptDiskSolid^ sd = dynamic_cast<IIfcSweptDiskSolid^>(solid);
			if (sd != nullptr) return Init(sd, logger);
			IIfcManifoldSolidBrep^ ms = dynamic_cast<IIfcManifoldSolidBrep^>(solid);
			if (ms != nullptr)
				return Init(ms, logger);
			IIfcCsgSolid^ csg = dynamic_cast<IIfcCsgSolid^>(solid);
			if (csg != nullptr)
			{
				XbimGeometryCreator::LogError(logger, csg, "The Init method for IIfcSolidModel should not be called with an IIfcCsgSolid. Use XbimSolidSet");
				return Init(csg, logger);
			}
			throw gcnew System::NotImplementedException(System::String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel));
		}

		void XbimSolid::Init(IIfcManifoldSolidBrep^ bRep, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(bRep, logger, _modelServices);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew(logger);
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
			}
		}


		void XbimSolid::Init(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcExtrudedAreaSolid^ extrudeArea = dynamic_cast<IIfcExtrudedAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea, overrideProfileDef, logger);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(solid);
			if (ras != nullptr) return Init(ras, overrideProfileDef, logger);
			IIfcFixedReferenceSweptAreaSolid^ fas = dynamic_cast<IIfcFixedReferenceSweptAreaSolid^>(solid);
			if (fas != nullptr) return Init(fas, overrideProfileDef, logger);
			IIfcSurfaceCurveSweptAreaSolid^ saw = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(solid);
			if (saw != nullptr) return Init(saw, overrideProfileDef, logger);
			XbimGeometryCreator::LogError(logger, solid, "Swept Solid of Type {ifcType} is not implemented", solid->GetType()->Name);
		}

		

		void XbimSolid::Init(IIfcRevolvedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger, _modelServices);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger, _modelServices);
			XbimFace^ faceEnd = gcnew XbimFace(repItem->EndSweptArea, logger, _modelServices);

			if (faceStart->IsValid && faceEnd->IsValid && repItem->Angle > 0) //we have a valid face and angle
			{
				IIfcAxis1Placement^ revolaxis = repItem->Axis;
				gp_Pnt origin(revolaxis->Location->X, revolaxis->Location->Y, revolaxis->Location->Z);
				XbimVector3D zDir = repItem->Axis->Z;
				gp_Dir vz(zDir.X, zDir.Y, zDir.Z);

				double radianConvert = repItem->Model->ModelFactors->AngleToRadiansConversionFactor;
				//create a curve trimmed to the 
				XbimPoint3D faceCentre = faceStart->Location;
				XbimPoint3D rotCentre(origin.X(), origin.Y(), origin.Z());
				XbimVector3D v = faceCentre - rotCentre;
				gp_Ax2 ax2(origin, vz, gp_Vec(v.X, v.Y, v.Z));
				gp_Circ circ(ax2, v.Length);
				double angle = System::Math::Min(repItem->Angle * radianConvert, M_PI * 2);;
				GC_MakeArcOfCircle arcMaker(circ, 0., angle, Standard_True);
				Handle(Geom_TrimmedCurve) trimmed = arcMaker.Value();
				XbimCurve^ curve = gcnew XbimCurve(trimmed, _modelServices);
				XbimEdge^ edge = gcnew XbimEdge(curve, _modelServices);
				XbimWire^ sweep = gcnew XbimWire(edge, _modelServices);
				//move the end face to the end position
				gp_Pnt ep; gp_Vec tan; gp_Vec norm;
				trimmed->D2(trimmed->LastParameter(), ep, tan, norm);
				gp_Ax3 toAx3(ep, tan, norm);	//rotate so normal of profile is tangental and X axis 
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				faceEnd->Move(trsf);
				TopoDS_Wire outerBoundStart = (XbimWire^)(faceStart->OuterBound);
				TopoDS_Wire outerBoundEnd = (XbimWire^)(faceEnd->OuterBound);
				int boundsCount = faceStart->InnerBounds->Count;
				TopTools_Array1OfShape wireStarts(1, boundsCount + 1);
				TopTools_Array1OfShape wireEnds(1, boundsCount + 1);
				for (int i = 1; i <= boundsCount; i++)
				{
					wireStarts(i) = faceStart->InnerWires->Wire[i];
					wireEnds(i) = faceEnd->InnerWires->Wire[i];
				}
				try
				{


					BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);


					//pipeMaker1.SetMode(Standard_True);

					// todo: all usages of BRepBuilderAPI_Transformed should be reviewed
					// it's possible that BRepBuilderAPI_RightCorner should be used in most cases
					// 
					pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
					pipeMaker1.Add(outerBoundStart);
					pipeMaker1.Add(outerBoundEnd);
					pipeMaker1.Build();
					if (pipeMaker1.IsDone())
					{

						TopoDS_Wire firstOuter = TopoDS::Wire(pipeMaker1.FirstShape().Reversed());
						TopoDS_Wire lastOuter = TopoDS::Wire(pipeMaker1.LastShape().Reversed());
						BRepBuilderAPI_MakeFace firstMaker(firstOuter);
						BRepBuilderAPI_MakeFace lastMaker(lastOuter);
						for (int i = 1; i <= boundsCount; i++)
						{
							//it is a hollow section so we need to build the inside
							BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
							TopoDS_Wire innerBoundStart = TopoDS::Wire(wireStarts(i));
							TopoDS_Wire innerBoundEnd = TopoDS::Wire(wireEnds(i));

							pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
							pipeMaker2.Add(innerBoundStart);
							pipeMaker2.Add(innerBoundEnd);
							pipeMaker2.Build();
							if (pipeMaker2.IsDone())
							{
								for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
								{
									b.AddShellFace(shell, TopoDS::Face(explr.Current().Reversed()));
								}
							}
							firstMaker.Add(TopoDS::Wire(pipeMaker2.FirstShape()));
							lastMaker.Add(TopoDS::Wire(pipeMaker2.LastShape()));
						}
						b.AddShellFace(shell, firstMaker.Face());
						b.AddShellFace(shell, lastMaker.Face());
						for (TopExp_Explorer explr(pipeMaker1.Shape(), TopAbs_FACE); explr.More(); explr.Next())
						{
							b.AddShellFace(shell, TopoDS::Face(explr.Current()));
						}
						TopoDS_Solid solid;
						BRep_Builder bs;
						bs.MakeSolid(solid);
						bs.Add(solid, shell);
						BRepClass3d_SolidClassifier sc(solid);
						sc.PerformInfinitePoint(Precision::Confusion());
						if (sc.State() == TopAbs_IN)
						{
							bs.MakeSolid(solid);
							shell.Reverse();
							bs.Add(solid, shell);

						}
						pSolid = new TopoDS_Solid();
						*pSolid = solid;
						pSolid->Closed(Standard_True);
						if (repItem->Position != nullptr) //In Ifc4 this is now optional
							pSolid->Move(XbimConvert::ToLocation(repItem->Position));
						ShapeFix_ShapeTolerance tolFixer;
						tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
						return;
					}
					else if (repItem->Angle <= 0)
					{
						XbimGeometryCreator::LogInfo(logger, repItem, "Invalid extrusion,  angle must be >0 ");
					}
				}
				catch (const Standard_Failure& sf)
				{
					System::String^ err = gcnew System::String(sf.GetMessageString());
					XbimGeometryCreator::LogWarning(logger, repItem, "Failed to create  IfcRevolvedAreaSolidTapered solid: " + err);
				}
				catch (...)
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Failed to create  IfcRevolvedAreaSolidTapered solid");
				}
				System::GC::KeepAlive(faceStart);
				System::GC::KeepAlive(faceEnd);

			}
		}

		void XbimSolid::Init(IIfcExtrudedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger, _modelServices);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger, _modelServices);
			XbimFace^ faceEnd = gcnew XbimFace(repItem->EndSweptArea, logger, _modelServices);

			if (faceStart->IsValid && faceEnd->IsValid && repItem->Depth > 0) //we have valid faces and extrusion
			{

				double precision = repItem->Model->ModelFactors->Precision;
				IIfcDirection^ dir = repItem->ExtrudedDirection;
				XbimVector3D vec(dir->X, dir->Y, dir->Z);
				vec = vec.Normalized();
				vec *= repItem->Depth;
				faceEnd->Translate(vec);
				try
				{
					BRepOffsetAPI_ThruSections pipeMaker(Standard_True, Standard_True, precision);
					TopoDS_Wire outerBoundStart = (XbimWire^)(faceStart->OuterBound);
					TopoDS_Wire outerBoundEnd = (XbimWire^)(faceEnd->OuterBound);
					pipeMaker.AddWire(outerBoundStart);
					pipeMaker.AddWire(outerBoundEnd);
					for (int i = 0; i < faceStart->InnerBounds->Count; i++)
					{
						TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
						faceEnd->InnerWires->Wire[i]->Translate(vec);
						TopoDS_Wire innerBoundEnd = faceEnd->InnerWires->Wire[i];
						pipeMaker.AddWire(innerBoundStart);
						pipeMaker.AddWire(innerBoundEnd);
					}
					pipeMaker.Build();
					if (pipeMaker.IsDone() && pipeMaker.Shape().ShapeType() == TopAbs_ShapeEnum::TopAbs_SOLID)
					{
						pSolid = new TopoDS_Solid();
						*pSolid = TopoDS::Solid(pipeMaker.Shape());
						pSolid->Closed(Standard_True);
						if (repItem->Position != nullptr) //In Ifc4 this is now optional
							pSolid->Move(XbimConvert::ToLocation(repItem->Position));
						ShapeFix_ShapeTolerance tolFixer;
						tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
						return;
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion, not a solid ");
					}
				}
				catch (const Standard_Failure& ex)
				{
					System::String^ err = gcnew System::String(ex.GetMessageString());
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion: " + err);
				}
				catch (...) //catch the access exceptions if raised and return gracefully
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion - ignored");
				}

				System::GC::KeepAlive(faceStart);
				System::GC::KeepAlive(faceEnd);
			}
			XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion, depth must be >0 and faces must be correctly defined");

		}

		void XbimSolid::Init(IIfcFixedReferenceSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger, _modelServices);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger, _modelServices);
			if (!faceStart->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build swept area");
				return;
			}

			IModelFactors^ mf = repItem->Model->ModelFactors;
			XbimWire^ sweep = gcnew XbimWire(repItem->Directrix, logger, _modelServices);

			if (dynamic_cast<IIfcLine^>(repItem->Directrix)) //params are different
			{
				IIfcLine^ line = (IIfcLine^)(repItem->Directrix);
				double mag = line->Dir->Magnitude;
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value * mag, repItem->EndParam.Value * mag, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value * mag, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value * mag, mf->Precision, logger);
			}
			else
			{
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, repItem->EndParam.Value, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value, mf->Precision, logger);
			}
			if (!sweep->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build directrix");
				return;
			}
			if (faceStart->IsValid) //we have valid faces and extrusion
			{

				IIfcDirection^ xdir = repItem->FixedReference;
				gp_Vec xVec(xdir->X, xdir->Y, XbimConvert::GetZValueOrZero(xdir));
				xVec.Normalize();

				TopoDS_Edge edge;
				Standard_Real uoe;
				BRepAdaptor_CompCurve cc(sweep);
				cc.Edge(0, edge, uoe);
				Standard_Real l, f;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
				//move the wire to the start point
				gp_Pnt p1;
				gp_Vec tangent;
				curve->D1(0, p1, tangent);

				gp_Ax3 toAx3(p1, tangent, xVec);	//rotate so normal of profile is tangental and X axis 
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				faceStart->SetLocation(topLoc);
				XbimWire^ outerBound = (XbimWire^)(faceStart->OuterBound);
				BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
				//pipeMaker1.SetMode(Standard_True);
				pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
				pipeMaker1.Add(outerBound);
				pipeMaker1.Build();
				if (pipeMaker1.IsDone())
				{
					TopoDS_Wire firstOuter = TopoDS::Wire(pipeMaker1.FirstShape());
					TopoDS_Wire lastOuter = TopoDS::Wire(pipeMaker1.LastShape().Reversed());

					BRepBuilderAPI_MakeFace firstMaker(firstOuter);
					BRepBuilderAPI_MakeFace lastMaker(lastOuter);
					for (int i = 0; i < faceStart->InnerBounds->Count; i++)
					{

						//it is a hollow section so we need to build the inside
						BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
						TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
						//pipeMaker2.SetMode(Standard_True);
						pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
						pipeMaker2.Add(innerBoundStart);
						pipeMaker2.Build();
						if (pipeMaker2.IsDone())
						{
							for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
							{
								b.AddShellFace(shell, TopoDS::Face(explr.Current()));
							}
						}
						firstMaker.Add(TopoDS::Wire(pipeMaker2.FirstShape()));
						lastMaker.Add(TopoDS::Wire(pipeMaker2.LastShape().Reversed()));
					}
					b.AddShellFace(shell, firstMaker.Face());
					b.AddShellFace(shell, lastMaker.Face());

					for (TopExp_Explorer explr(pipeMaker1.Shape(), TopAbs_FACE); explr.More(); explr.Next())
					{
						b.AddShellFace(shell, TopoDS::Face(explr.Current()));
					}
					b.CompleteShell(shell);
					ShapeFix_Shell sf(shell);
					if (sf.FixFaceOrientation(shell))
					{
						shell = sf.Shell();
					}
					TopoDS_Solid solid;
					BRep_Builder bs;
					bs.MakeSolid(solid);
					bs.Add(solid, shell);
					BRepClass3d_SolidClassifier sc(solid);
					sc.PerformInfinitePoint(Precision::Confusion());
					if (sc.State() == TopAbs_IN)
					{
						bs.MakeSolid(solid);
						shell.Reverse();
						bs.Add(solid, shell);

					}
					pSolid = new TopoDS_Solid();
					*pSolid = solid;
					pSolid->Closed(Standard_True);
					if (repItem->Position != nullptr) //In Ifc4 this is now optional
						pSolid->Move(XbimConvert::ToLocation(repItem->Position));
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
					return;
				}
				System::GC::KeepAlive(faceStart);

			}
			XbimGeometryCreator::LogInfo(logger, repItem, "Invalid extrusion, depth must be >0 and faces must be correctly defined");
			//if it has failed we will have a null solid

		}

		void XbimSolid::Init(IIfcSectionedSpine^ repItem, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);

			//make a list of faces for each cross section (wire is not good enough as there ay be holes to consider
			List<XbimFace^>^ crossSections = gcnew List<XbimFace^>();
			for each (IIfcProfileDef ^ profile in repItem->CrossSections)
			{
				XbimFace^ f = gcnew XbimFace(profile, logger, _modelServices);
				crossSections->Add(f);
			}
			List<IIfcAxis2Placement3D^>^ positions = Enumerable::ToList<IIfcAxis2Placement3D^>(repItem->CrossSectionPositions);

			if (crossSections->Count > 1) //we have valid faces 
			{

				//build the spine
				XbimWire^ sweep = gcnew XbimWire(repItem->SpineCurve, logger, _modelServices);
				BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
				pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
				//move the sections to the right position
				for (int i = 0; i < crossSections->Count; i++)
				{
					crossSections[i]->Move(positions[i]);
					TopoDS_Wire outerBound = (XbimWire^)(crossSections[i]->OuterBound);
					pipeMaker1.Add(outerBound);
				}
				bool ok = false;
				try
				{
					pipeMaker1.Build();
					ok = true;
				}
				catch (...)
				{
				}
				if (!ok)
				{
					BRepBuilderAPI_PipeError err = pipeMaker1.GetStatus();
					XbimGeometryCreator::LogWarning(logger, repItem, "Failed to build outer shell of sectioned spine {occError}", (int)err);
					return;
				}
				if (pipeMaker1.IsDone())
				{
					if (!pipeMaker1.MakeSolid())
					{
						XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct IfcSectionedSpine");
						return;
					}
					TopoDS_Solid outerShell = TopoDS::Solid(pipeMaker1.Shape());

					for (int i = 0; i < crossSections[0]->InnerBounds->Count; i++) //assume all sections have same topology
					{
						//it is a hollow section so we need to build the inside
						BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
						pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
						for (int j = 0; j < crossSections->Count; j++)
						{
							TopoDS_Wire innerBound = (XbimWire^)(crossSections[j]->InnerWires->Wire[i]);
							pipeMaker2.Add(innerBound);
						}
						ok = false;
						try
						{
							pipeMaker2.Build();
							ok = true;
						}
						catch (...)
						{

						}
						if (!ok)
						{
							BRepBuilderAPI_PipeError err = pipeMaker2.GetStatus();
							XbimGeometryCreator::LogWarning(logger, repItem, "Failed to build inner shell of sectioned spine {occError}", (int)err);
							return;
						}
						if (pipeMaker2.IsDone())
						{
							if (!pipeMaker2.MakeSolid()) //we cannot make a solid
							{
								XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct inner sweep of IfcSectionedSpine");
							}
							else
							{
								TopoDS_Solid innerShell = TopoDS::Solid(pipeMaker2.Shape());
								TopTools_IndexedMapOfShape outerShellMap;
								TopExp::MapShapes(outerShell, TopAbs_FACE, outerShellMap);
								TopoDS_Face firstFace;
								TopoDS_Face lastFace;
								for (int k = 1; k <= outerShellMap.Extent(); k++)
								{
									const TopoDS_Face& f = TopoDS::Face(outerShellMap(k));
									if (f.IsSame(pipeMaker1.FirstShape()))
										firstFace = f;
									else if (f.IsSame(pipeMaker1.LastShape()))
										lastFace = f;
									else
										b.AddShellFace(shell, f);
								}
								TopTools_IndexedMapOfShape innerShellMap;
								TopExp::MapShapes(innerShell, TopAbs_FACE, innerShellMap);
								for (int k = 1; k <= innerShellMap.Extent(); k++)
								{
									const TopoDS_Face& f = TopoDS::Face(innerShellMap(k));

									if (f.IsEqual(pipeMaker2.FirstShape()))
									{
										BRepBuilderAPI_MakeFace firstMaker(firstFace);
										TopoDS_Wire w = BRepTools::OuterWire(f);
										w.Reverse();
										firstMaker.Add(w);
										firstFace = firstMaker.Face();
									}
									else if (f.IsEqual(pipeMaker2.LastShape()))
									{
										BRepBuilderAPI_MakeFace lastMaker(lastFace);
										TopoDS_Wire w = BRepTools::OuterWire(f);
										w.Reverse();
										lastMaker.Add(w);
										lastFace = lastMaker.Face();
									}
									else
										b.AddShellFace(shell, TopoDS::Face(f.Reversed()));

								}
								TopoDS_Solid solid;
								BRep_Builder bs;
								bs.MakeSolid(solid);

								bs.Add(shell, firstFace);
								bs.Add(shell, lastFace);
								bs.Add(solid, shell);

								BRepClass3d_SolidClassifier SC(solid);
								SC.PerformInfinitePoint(Precision::Confusion());
								if (SC.State() == TopAbs_IN) {
									bs.MakeSolid(solid);
									shell.Reverse();
									bs.Add(solid, shell);
								}
								solid.Closed(Standard_True);
								outerShell = solid;
							}
						}
						else
						{
							XbimGeometryCreator::LogWarning(logger, repItem, "Inner loop of IfcSectionedSpine could not be constructed");
						}

					}

					pSolid = new TopoDS_Solid();
					*pSolid = outerShell;
					pSolid->Closed(Standard_True);
					System::GC::KeepAlive(crossSections);
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
					return;
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "IfcSectionedSpine ould not be constructed");

				}
			}
			XbimGeometryCreator::LogInfo(logger, repItem, "Invalid IfcSectionedSpine, sections must be 2 or more and faces must be correctly defined");
			//if it has failed we will have a null solid
		}

		void XbimSolid::Init(IIfcSurfaceCurveSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			pSolid = new TopoDS_Solid(TopoDS::Solid(_modelServices->GetSolidFactory()->BuildSurfaceCurveSweptAreaSolid(repItem, overrideProfileDef)));
		}

		void XbimSolid::Init(IIfcExtrudedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcExtrudedAreaSolidTapered^ extrudeTaperedArea = dynamic_cast<IIfcExtrudedAreaSolidTapered^>(repItem);
			if (extrudeTaperedArea != nullptr) 
				return Init(extrudeTaperedArea, overrideProfileDef, logger);

			pSolid = new TopoDS_Solid(TopoDS::Solid(_modelServices->GetSolidFactory()->BuildExtrudedAreaSolid(repItem, overrideProfileDef)));
		}

		void XbimSolid::Init(IIfcRevolvedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcRevolvedAreaSolidTapered^ extrudeTaperedArea = dynamic_cast<IIfcRevolvedAreaSolidTapered^>(repItem);
			if (extrudeTaperedArea != nullptr) return Init(extrudeTaperedArea, overrideProfileDef, logger);
			XbimFace^ face;
			if (overrideProfileDef == nullptr)
				face = gcnew XbimFace(repItem->SweptArea, logger, _modelServices);
			else
				face = gcnew XbimFace(overrideProfileDef, logger, _modelServices);

			if (face->IsValid && repItem->Angle > 0) //we have a valid face and angle
			{
				IIfcAxis1Placement^ revolaxis = repItem->Axis;
				gp_Pnt origin(
					revolaxis->Location->X,
					revolaxis->Location->Y,
					XbimConvert::GetZValueOrZero(revolaxis->Location)
				);
				XbimVector3D zDir = revolaxis->Z;
				gp_Dir vx(zDir.X, zDir.Y, zDir.Z);
				gp_Ax1 ax1(origin, vx);
				double radianConvert = repItem->Model->ModelFactors->AngleToRadiansConversionFactor;
				BRepPrimAPI_MakeRevol revol(face, ax1, repItem->Angle * radianConvert);

				System::GC::KeepAlive(face);
				if (revol.IsDone())
				{
					//BRepTools::Write(revol.Shape(), "d:\\tmp\\rev");
					pSolid = new TopoDS_Solid();
					if (repItem->Position != nullptr)
						*pSolid = TopoDS::Solid(revol.Shape());
					pSolid->Move(XbimConvert::ToLocation(repItem->Position));
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
				}
				else
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalidextrusion, could not create solid");
			}
			else if (repItem->Angle <= 0)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Invalidextrusion, angle must be >0");
			}
		}

		void XbimSolid::Init(IIfcHalfSpaceSolid^ hs, ILogger^ logger)
		{
			auto solid = _modelServices->GetSolidFactory()->BuildHalfSpace(hs); //throws an exception on failure
			pSolid = new TopoDS_Solid();
			*pSolid = solid;
		}

		void XbimSolid::Init(XbimRect3D rect3D, double tolerance, ILogger^ /*logger*/)
		{

			XbimPoint3D l = rect3D.Location;
			BRepPrimAPI_MakeBox box(gp_Pnt(l.X, l.Y, l.Z), rect3D.SizeX, rect3D.SizeY, rect3D.SizeZ);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(box.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, tolerance);
		}


		// params depend on segment type
		double XbimSolid::SegLength(IIfcCompositeCurveSegment^ segment, ILogger^ logger)
		{
			IIfcLine^ line = dynamic_cast<IIfcLine^>(segment->ParentCurve);
			IIfcTrimmedCurve^ trimmedCurve = dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve);
			if (line != nullptr)
			{
				//srl perhaps it should be this to obey IFC parametric rules
				//return line->Dir->Magnitude;
				return 1;
			}
			else if (trimmedCurve != nullptr)
			{
				try {
					IIfcTrimmedCurve^ tc = dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve);
					double valTrim1 = 0;
					double valTrim2 = 1;
					// search for parameter values
					Xbim::Ifc4::MeasureResource::IfcParameterValue^ t;

					for (int i = 0; i < tc->Trim1->Count; i++)
					{
						t = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim1[i]);
						if (t && t->Value)
						{
							valTrim1 = (double)t->Value;
							break;
						}
					}
					for (int i = 0; i < tc->Trim2->Count; i++)
					{
						t = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim2[i]);
						if (t && t->Value)
						{
							valTrim2 = (double)t->Value;
							break;
						}
					}
					double ret = valTrim2 - valTrim1;

					if (ret < 0 && (dynamic_cast<IIfcConic^>(tc->BasisCurve) != nullptr)) //params will be periodic so take the abs length
						ret = System::Math::Abs(ret);
					if (ret < 0)
					{
						XbimGeometryCreator::LogWarning(logger, segment, "Negative length value. Returned 1.");
						return 1;
					}
					return ret;
				}
				catch (System::Exception^ e) {
					XbimGeometryCreator::LogWarning(logger, segment, "Could not compute segment parametric length. Returned 1." + e->Message);
					return 1;
				}
			}
			return 1;
		}

		void XbimSolid::Init(IIfcSweptDiskSolid^ repItem, ILogger^ logger)
		{
			pSolid = new TopoDS_Solid(_modelServices->GetSolidFactory()->BuildSweptDiskSolid(repItem));
		}


		void XbimSolid::Init(IIfcBoundingBox^ box, ILogger^ /*logger*/)
		{
			double precision = box->Model->ModelFactors->Precision;
			double x = System::Math::Max(box->XDim, precision);
			double y = System::Math::Max(box->YDim, precision);
			double z = System::Math::Max(box->ZDim, precision);
			gp_Ax2 	gpax2(gp_Pnt(box->Corner->X, box->Corner->Y, box->Corner->Z), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
			BRepPrimAPI_MakeBox boxMaker(gpax2, x, y, z);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
			ShapeFix_ShapeTolerance FTol;
			FTol.LimitTolerance(*pSolid, box->Model->ModelFactors->Precision);
		}



		void XbimSolid::Init(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^ logger)
		{
			IIfcSphere^ sphere = dynamic_cast<IIfcSphere^>(IIfcSolid);
			if (sphere != nullptr) return Init(sphere, logger);
			IIfcBlock^ block = dynamic_cast<IIfcBlock^>(IIfcSolid);
			if (block != nullptr) return Init(block, logger);
			IIfcRightCircularCylinder^ cylinder = dynamic_cast<IIfcRightCircularCylinder^>(IIfcSolid);
			if (cylinder != nullptr) return Init(cylinder, logger);
			IIfcRightCircularCone^ cone = dynamic_cast<IIfcRightCircularCone^>(IIfcSolid);
			if (cone != nullptr) return Init(cone, logger);
			IIfcRectangularPyramid^ pyramid = dynamic_cast<IIfcRectangularPyramid^>(IIfcSolid);
			if (pyramid != nullptr) return Init(pyramid, logger);
			throw gcnew System::NotImplementedException(System::String::Format("IIfcCsgPrimitive3D of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));
		}



		void XbimSolid::Init(IIfcSphere^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeSphere sphereMaker(gpax3.Ax2(), IIfcSolid->Radius);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(sphereMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcBlock^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeBox boxMaker(gpax3.Ax2(), IIfcSolid->XLength, IIfcSolid->YLength, IIfcSolid->ZLength);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRightCircularCylinder^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeCylinder cylinderMaker(gpax3.Ax2(), IIfcSolid->Radius, IIfcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(cylinderMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRightCircularCone^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeCone coneMaker(gpax3.Ax2(), IIfcSolid->BottomRadius, 0., IIfcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(coneMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRectangularPyramid^ IIfcSolid, ILogger^ /*logger*/)
		{

			double xOff = IIfcSolid->XLength / 2;
			double yOff = IIfcSolid->YLength / 2;
			double precision = IIfcSolid->Model->ModelFactors->Precision;

			gp_Pnt bl(0, 0, 0);
			gp_Pnt br(IIfcSolid->XLength, 0, 0);
			gp_Pnt tr(IIfcSolid->XLength, IIfcSolid->YLength, 0);
			gp_Pnt tl(0, IIfcSolid->YLength, 0);
			gp_Pnt p(xOff, yOff, IIfcSolid->Height);
			//make the vertices
			BRep_Builder builder;
			TopoDS_Vertex vbl, vbr, vtr, vtl, vp;
			TopoDS_Shell shell;
			builder.MakeShell(shell);
			builder.MakeVertex(vbl, bl, precision);
			builder.MakeVertex(vbr, br, precision);
			builder.MakeVertex(vtr, tr, precision);
			builder.MakeVertex(vtl, tl, precision);
			builder.MakeVertex(vp, p, precision);
			//make the edges
			TopoDS_Wire baseWire;
			builder.MakeWire(baseWire);
			const TopoDS_Edge& brbl = BRepBuilderAPI_MakeEdge(vbr, vbl);
			const TopoDS_Edge& trbr = BRepBuilderAPI_MakeEdge(vtr, vbr);
			const TopoDS_Edge& tltr = BRepBuilderAPI_MakeEdge(vtl, vtr);
			const TopoDS_Edge& bltl = BRepBuilderAPI_MakeEdge(vbl, vtl);
			builder.Add(baseWire, brbl);
			builder.Add(baseWire, bltl);
			builder.Add(baseWire, tltr);
			builder.Add(baseWire, trbr);
			BRepBuilderAPI_MakeFace afaceBlder(gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, -1)), baseWire, Standard_True);
			builder.Add(shell, afaceBlder.Face());
			//build the sides
			const TopoDS_Edge& blp = BRepBuilderAPI_MakeEdge(vbl, vp);
			const TopoDS_Edge& tlp = BRepBuilderAPI_MakeEdge(vtl, vp);
			const TopoDS_Edge& brp = BRepBuilderAPI_MakeEdge(vbr, vp);
			const TopoDS_Edge& trp = BRepBuilderAPI_MakeEdge(vtr, vp);
			TopoDS_Wire bltlWire;
			builder.MakeWire(bltlWire);
			builder.Add(bltlWire, bltl.Reversed());
			builder.Add(bltlWire, blp);
			builder.Add(bltlWire, tlp.Reversed());
			BRepBuilderAPI_MakeFace bfaceBlder(bltlWire, Standard_True);
			builder.Add(shell, bfaceBlder.Face());

			TopoDS_Wire tltrWire;
			builder.MakeWire(tltrWire);
			builder.Add(tltrWire, tltr.Reversed());
			builder.Add(tltrWire, tlp);
			builder.Add(tltrWire, trp.Reversed());
			BRepBuilderAPI_MakeFace cfaceBlder(tltrWire, Standard_True);
			builder.Add(shell, cfaceBlder.Face());


			TopoDS_Wire brtlWire;
			builder.MakeWire(brtlWire);
			builder.Add(brtlWire, trbr.Reversed());
			builder.Add(brtlWire, trp);
			builder.Add(brtlWire, brp.Reversed());
			BRepBuilderAPI_MakeFace dfaceBlder(brtlWire, Standard_True);
			builder.Add(shell, dfaceBlder.Face());

			TopoDS_Wire blbrWire;
			builder.MakeWire(blbrWire);
			builder.Add(blbrWire, brbl.Reversed());
			builder.Add(blbrWire, brp);
			builder.Add(blbrWire, blp.Reversed());
			BRepBuilderAPI_MakeFace efaceBlder(blbrWire, Standard_True);
			builder.Add(shell, efaceBlder.Face());

			BRepBuilderAPI_MakeSolid solidMaker(shell);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(solidMaker.Shape());
			Move(IIfcSolid->Position);
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
		}

		void XbimSolid::Init(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(IIfcSolid, logger, _modelServices);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew(logger);
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
			}
		}

		void XbimSolid::Init(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew(logger);
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
			}
		}

		void XbimSolid::Init(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew(logger);
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, _modelServices->MinimumGap);
			}
		}

		TopoDS_Face BuildTriangularFace(const TopoDS_Edge& base, const TopoDS_Vertex& l, const TopoDS_Vertex& r, const TopoDS_Vertex& t)
		{
			BRep_Builder builder;
			//build the sides
			const TopoDS_Edge& rt = BRepBuilderAPI_MakeEdge(r, t);
			const TopoDS_Edge& tl = BRepBuilderAPI_MakeEdge(t, l);
			TopoDS_Wire bltlWire;
			builder.MakeWire(bltlWire);
			builder.Add(bltlWire, base);
			builder.Add(bltlWire, rt);
			builder.Add(bltlWire, tl);
			BRepBuilderAPI_MakeFace faceBlder(bltlWire);
			return faceBlder.Face();
		}
#pragma endregion

#pragma region IXbimSolid Interface

		IXbimShellSet^ XbimSolid::Shells::get()
		{
			if (!IsValid) return gcnew XbimShellSet(_modelServices);
			return gcnew XbimShellSet(*pSolid, _modelServices);
		}


		IXbimFaceSet^ XbimSolid::Faces::get()
		{
			if (!IsValid) return gcnew XbimFaceSet(_modelServices);
			return gcnew XbimFaceSet(*pSolid, _modelServices);
		}

		IXbimEdgeSet^ XbimSolid::Edges::get()
		{
			if (!IsValid) return gcnew XbimEdgeSet(_modelServices);
			return gcnew XbimEdgeSet(*pSolid, _modelServices);
		}
		IXbimVertexSet^ XbimSolid::Vertices::get()
		{
			if (!IsValid) return gcnew XbimVertexSet(_modelServices);
			return gcnew XbimVertexSet(*pSolid, _modelServices);
		}

		XbimRect3D XbimSolid::BoundingBox::get()
		{
			if (pSolid == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			if (IsPolyhedron)
				BRepBndLib::AddClose(*pSolid, pBox);
			else
				BRepBndLib::Add(*pSolid, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			System::GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		//returns true if the solid is a closed manifold typically with one shell, if there are more shells they are voids and should also be closed
		bool XbimSolid::IsClosed::get()
		{
			if (!IsValid) return false;
			return pSolid->Closed();
		}

		bool XbimSolid::IsEmpty::get()
		{
			if (!IsValid) return true;
			TopTools_IndexedMapOfShape shellMap;
			TopExp::MapShapes(*pSolid, TopAbs_SHELL, shellMap);
			if (shellMap.Extent() == 0) return true;
			for (int i = 1; i <= shellMap.Extent(); i++)
			{
				TopTools_IndexedMapOfShape faceMap;
				TopExp::MapShapes(TopoDS::Shell(shellMap(i)), TopAbs_FACE, faceMap);
				if (faceMap.Extent() > 0) return false; //if we find a face in a shell we have something to work with
			}
			return true;
		}

		double XbimSolid::Volume::get()
		{
			if (IsValid)
			{
				try
				{
					GProp_GProps gProps;
					TopoDS_Solid solid = *pSolid;
					BRepGProp::VolumeProperties(solid, gProps);
					return gProps.Mass();
				}
				catch (const Standard_Failure& sf)
				{
					System::String^ err = gcnew System::String(sf.GetMessageString());
					_modelServices->LogError("Error calculating XbimSolid volume " + err);
					return 0;
				}
			}
			else
				return 0;
		}

		bool XbimSolid::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for (TopExp_Explorer exp(*pSolid, TopAbs_FACE); exp.More(); exp.Next())
			{
				Handle(Geom_Surface) s = BRep_Tool::Surface(TopoDS::Face(exp.Current()));
				GeomLib_IsPlanarSurface tester(s);
				if (!tester.IsPlanar())
					return false;
			}
			System::GC::KeepAlive(this);
			//all faces are planar
			return true;
		}



		double XbimSolid::SurfaceArea::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::SurfaceProperties(*pSolid, gProps);
				System::GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}


		bool XbimSolid::HasValidTopology::get()
		{
			if (!IsValid) return false;
			BRepCheck_Analyzer analyser(*pSolid, Standard_True);
			System::GC::KeepAlive(this);
			return analyser.IsValid() == Standard_True;
		}


		IXbimGeometryObject^ XbimSolid::Transform(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_True);
			System::GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()), _modelServices);
		}

		IXbimGeometryObject^ XbimSolid::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_False);
			System::GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()), _modelServices);
		}

		IXbimSolidSet^ XbimSolid::Cut(IXbimSolidSet^ toCut, double tolerance, ILogger^ logger)
		{
			if (toCut->Count == 0) return gcnew XbimSolidSet(this, _modelServices);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this, _modelServices);
			return thisSolidSet->Cut(toCut, tolerance, logger);
		}


		IXbimSolidSet^ XbimSolid::Cut(IXbimSolid^ toCut, double tolerance, ILogger^ logger)
		{

			TopoDS_Solid right = static_cast<XbimSolid^>(toCut);
			TopoDS_Solid left = *pSolid;
			bool hasWarnings;
			TopoDS_Shape resultShape = _modelServices->GetBooleanFactory()->EXEC_NATIVE->Cut(left, right, tolerance, hasWarnings);
			if (hasWarnings)
				XbimGeometryCreator::LogWarning(logger, nullptr, "Solid entity cut operation has generated warnings. See logs");
			return gcnew XbimSolidSet(resultShape, _modelServices);

		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolidSet^ toIntersect, double tolerance, ILogger^ logger)
		{
			if (toIntersect->Count == 0) return gcnew XbimSolidSet(this, _modelServices);
			if (toIntersect->Count == 1) return this->Intersection(toIntersect->First, tolerance, logger);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this, _modelServices);
			return thisSolidSet->Intersection(toIntersect, tolerance, logger);
		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolid^ toIntersect, double tolerance, ILogger^ logger)
		{

			TopoDS_Solid right = static_cast<XbimSolid^>(toIntersect);
			TopoDS_Solid left = *pSolid;
			bool hasWarnings;
			TopoDS_Shape resultShape = _modelServices->GetBooleanFactory()->EXEC_NATIVE->Intersect(left, right, tolerance, hasWarnings);
			if (hasWarnings)
				XbimGeometryCreator::LogWarning(logger, nullptr, "Solid entity intersect operation has generated warnings. See logs");
			return gcnew XbimSolidSet(resultShape, _modelServices);
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolidSet^ toUnion, double tolerance, ILogger^ logger)
		{
			if (toUnion->Count == 0) return gcnew XbimSolidSet(this, _modelServices);
			if (toUnion->Count == 1) return this->Union(toUnion->First, tolerance, logger);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this, _modelServices);
			return thisSolidSet->Union(toUnion, tolerance, logger);
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolid^ toUnion, double tolerance, ILogger^ logger)
		{
			TopoDS_Solid right = static_cast<XbimSolid^>(toUnion);
			TopoDS_Solid left = *pSolid;
			bool hasWarnings;
			TopoDS_Shape resultShape = _modelServices->GetBooleanFactory()->EXEC_NATIVE->Union(left, right, tolerance, hasWarnings);
			if (hasWarnings)
				XbimGeometryCreator::LogWarning(logger, nullptr, "Solid entity union operation has generated warnings. See logs");
			return gcnew XbimSolidSet(resultShape, _modelServices);
		}


		IXbimFaceSet^ XbimSolid::Section(IXbimFace^ toSection, double tolerance, ILogger^ logger)
		{
			if (!IsValid || !toSection->IsValid) return gcnew XbimFaceSet(_modelServices);
			XbimFace^ faceSection = dynamic_cast<XbimFace^>(toSection);
			if (faceSection == nullptr)  throw gcnew System::ArgumentException("Only IXbimSolids created by Xbim.OCC modules are supported", "toSection");

			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(faceSection, tolerance);
			fixTol.SetTolerance(this, tolerance);
			BRepAlgoAPI_Section boolOp(this, faceSection, false);
			boolOp.ComputePCurveOn2(Standard_True);
			boolOp.Build();

			if (boolOp.IsDone())
			{
				Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape();
				Handle(TopTools_HSequenceOfShape) wires = new TopTools_HSequenceOfShape();
				for (TopExp_Explorer expl(boolOp.Shape(), TopAbs_EDGE); expl.More(); expl.Next())
					edges->Append(TopoDS::Edge(expl.Current()));

				TopoDS_Compound open;
				TopoDS_Compound closed;
				BRep_Builder b;
				b.MakeCompound(open);
				b.MakeCompound(closed);

				ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance, false, wires);
				ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
				TopExp_Explorer exp(closed, TopAbs_WIRE);
				if (!exp.More()) //try and resolve precision errors
				{
					wires->Clear();
					ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance * 10, false, wires);
					ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
					TopExp_Explorer exp2(closed, TopAbs_WIRE);
					if (!exp2.More())
					{
						wires->Clear();
						ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance * 100, false, wires);
						ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
					}
				}

				BRepAlgo_FaceRestrictor fr;
				TopoDS_Shape aLocalS = boolOp.Shape2().Oriented(TopAbs_FORWARD);
				fr.Init(TopoDS::Face(aLocalS), Standard_True, Standard_True);
				for (TopExp_Explorer exp2(closed, TopAbs_WIRE); exp2.More(); exp2.Next())
				{
					ShapeFix_Wire wireFixer(TopoDS::Wire(exp2.Current()), faceSection, tolerance);
					wireFixer.Perform();
					TopoDS_Wire w = wireFixer.Wire();
					fr.Add(w);
				}
				fr.Perform();
				if (fr.IsDone())
				{
					TopTools_ListOfShape result;
					TopAbs_Orientation orientationOfFace = boolOp.Shape2().Orientation();
					for (; fr.More(); fr.Next())
					{
						result.Append(TopoDS::Face(fr.Current().Oriented(orientationOfFace)));
					}

					return gcnew XbimFaceSet(result, _modelServices);
				}
				System::GC::KeepAlive(faceSection);
				System::GC::KeepAlive(toSection);
				System::GC::KeepAlive(this);
			}
			XbimGeometryCreator::LogWarning(logger, toSection, "Boolean Section operation has failed to create a section");

			return gcnew XbimFaceSet(_modelServices);
		}

		void XbimSolid::SaveAsBrep(System::String^ fileName)
		{
			if (IsValid)
			{
				XbimOccWriter^ occWriter = gcnew XbimOccWriter();
				occWriter->Write(this, fileName);
			}
		}

#pragma endregion

#pragma region Boolean operations

		void XbimSolid::Move(TopLoc_Location loc)
		{
			if (IsValid) pSolid->Move(loc);
		}

		void XbimSolid::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
			pSolid->Move(toPos);
		}

		void XbimSolid::Translate(XbimVector3D translation)
		{
			if (!IsValid) return;
			gp_Vec v(translation.X, translation.Y, translation.Z);
			gp_Trsf t;
			t.SetTranslation(v);
			pSolid->Move(t);
		}

		void XbimSolid::Reverse()
		{
			if (!IsValid) return;
			pSolid->Reverse();
		}

		void XbimSolid::CorrectOrientation()
		{
			if (IsValid)
			{
				BRepClass3d_SolidClassifier class3d(*pSolid);
				class3d.PerformInfinitePoint(Precision::Confusion());
				if (class3d.State() == TopAbs_IN) this->Reverse();
			}
		}



		bool XbimSolid::FixTopology(double tolerance)
		{
			if (IsValid)
			{

				if (BRepCheck_Analyzer(*pSolid, Standard_True).IsValid() == Standard_False)
				{
					ShapeFix_Shape shapeFixer(*pSolid);
					shapeFixer.SetPrecision(tolerance * 10);
					shapeFixer.LimitTolerance(tolerance * 10);
					shapeFixer.FixSolidMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixIntersectingWiresMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixOrientationMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixAddCurve3dMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixIntersectingEdgesMode() = Standard_True;
					Handle(ShapeFix_Solid) solidTool = shapeFixer.FixSolidTool();
					if (shapeFixer.Perform())
					{
						//	if (BRepCheck_Analyzer(shapeFixer.Shape(), Standard_True).IsValid() == Standard_True)
						{
							if (solidTool->Solid().ShapeType() == TopAbs_SOLID)
							{
								*pSolid = TopoDS::Solid(solidTool->Solid());
								//ShapeUpgrade_UnifySameDomain unifier(ts);
								//unifier.SetAngularTolerance(0.00174533); //1 tenth of a degree
								//unifier.SetLinearTolerance(tolerance);
								//try
								//{
								//	//sometimes unifier crashes
								//	unifier.Build();
								//	if (unifier.Shape().ShapeType() == TopAbs_SOLID)
								//	{
								//		*pSolid = (TopoDS::Solid(unifier.Shape()));
								//		return true; //only place for total success, all else is a failure
								//	}
								//}
								//catch (...) //just catch its a fail
								//{
								//}
							}
						}
					}

				}
				else
					return true;
			}
			return false;
		}

		XbimGeometryObject^ XbimSolid::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(*pSolid, trans, Standard_True); //make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimSolid(TopoDS::Solid(tr.Shape()), Tag, _modelServices);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(*pSolid, trans, Standard_False); //do not make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimSolid(TopoDS::Solid(tr.Shape()), Tag, _modelServices);
			}
		}

		XbimGeometryObject^ XbimSolid::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimSolid^ copy = gcnew XbimSolid(*pSolid, Tag, _modelServices); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject^ XbimSolid::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimSolid^ copy = gcnew XbimSolid(*pSolid, Tag, _modelServices); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger, _modelServices);
			copy->Move(loc);
			return copy;
		}


#pragma endregion
	}


}