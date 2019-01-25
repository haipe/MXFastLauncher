#pragma once

namespace commonutil
{

	template<typename ObjType>
	class ObjContainer
	{
	public:
		typedef ObjType* ObjPtr;
		ObjContainer( ObjPtr obj = nullptr ) :m_obj_ptr( obj ){}

		void SetObj( ObjPtr obj )
		{
			m_obj_ptr = obj;
		}

		ObjPtr GetObj()
		{
			return m_obj_ptr;
		}
	protected:
		ObjPtr	m_obj_ptr;
	};
}