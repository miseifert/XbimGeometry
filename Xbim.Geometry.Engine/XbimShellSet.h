#pragma once
#include "XbimShell.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimShellSet : IXbimShellSet
		{
		private:
			List<IXbimShell^>^ shells;
			static XbimShellSet^ empty = gcnew XbimShellSet();
			XbimShellSet::XbimShellSet(){ shells = gcnew List<IXbimShell^>(1); }
			void InstanceCleanup()
			{
				shells = nullptr;
			};
		public:
			static property XbimShellSet^ Empty{XbimShellSet^ get(){ return empty; }};

#pragma region Constructors

			XbimShellSet(const TopoDS_Shape& shape);
			XbimShellSet(List<IXbimShell^>^ shells);

#pragma endregion
#pragma region destructors

			~XbimShellSet(){ InstanceCleanup(); }
			!XbimShellSet(){ InstanceCleanup(); }

#pragma endregion
#pragma region IXbimShellSet interface

			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimShell^ First{IXbimShell^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimShellSetType; }}
			virtual IEnumerator<IXbimShell^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual void Add(IXbimGeometryObject^ solid);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual property bool IsPolyhedron{ bool get(); }
			virtual void Union(double tolerance);
#pragma endregion

		};

	}
}

