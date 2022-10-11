
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include "XbimVertexSet.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>

namespace Xbim
{
	namespace Geometry
	{
		XbimVertexSet::operator  TopoDS_Shape ()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimVertex ^ vertex in vertices)
			{
				builder.Add(bodyCompound, (XbimVertexV5^) vertex);
			}
			return bodyCompound;
		}
#pragma region Constructors
		


		XbimVertexSet::XbimVertexSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_VERTEX, map);
			vertices = gcnew  List<IXbimVertex^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				vertices->Add(gcnew XbimVertexV5(TopoDS::Vertex(map(i))));
		}

		XbimVertexSet::XbimVertexSet(IEnumerable<IXbimVertex^>^ vertices)
		{
			this->vertices = gcnew List<IXbimVertex^>(vertices);
		}

#pragma endregion


		IXbimVertex^ XbimVertexSet::First::get()
		{
			if (vertices->Count == 0) return nullptr;
			return vertices[0];
		}

		int XbimVertexSet::Count::get()
		{
			return vertices == nullptr ? 0 : vertices->Count;
		}

		IXbimGeometryObject^ XbimVertexSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimVertex^>^ result = gcnew List<IXbimVertex^>(vertices->Count);
			for each (IXbimGeometryObject^ vertex in vertices)
			{
				result->Add((IXbimVertex^)vertex->Transform(matrix3D));
			}
			return gcnew XbimVertexSet(result);
		}

		IXbimGeometryObject^ XbimVertexSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimVertex^>^ result = gcnew List<IXbimVertex^>(vertices->Count);
			for each (IXbimGeometryObject^ vertex in vertices)
			{
				result->Add((IXbimVertex^)((XbimVertexV5^)vertex)->TransformShallow(matrix3D));
			}
			return gcnew XbimVertexSet(result);
		}

		IXbimGeometryObject ^ XbimVertexSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet();
			for each (XbimVertexV5^ vertex in vertices)
				result->Add((XbimVertexV5^)vertex->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimVertexSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimVertex^ vertex in vertices)
			{
				XbimVertexV5^ copy = gcnew XbimVertexV5((XbimVertexV5^)vertex, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimVertexSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger )
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			for each (IXbimVertex^ vertex in vertices)
			{
				XbimVertexV5^ copy = gcnew XbimVertexV5((XbimVertexV5^)vertex, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimVertexSet::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/, double /*deflection*/, double /*angle*/)
		{
			throw gcnew System::NotImplementedException("XbimVertexSet::Mesh");
		}


		XbimRect3D XbimVertexSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in vertices)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IEnumerator<IXbimVertex^>^ XbimVertexSet::GetEnumerator()
		{
			return vertices->GetEnumerator();
		}
	}
}
