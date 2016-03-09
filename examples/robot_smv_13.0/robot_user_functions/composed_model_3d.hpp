/*
This file is part of SMV.

History:

* 2006.03.22 - Created by Helio Perroni Filho
*/

#ifndef __MAE_SMV_COMPOSED_MODEL_3D_HPP
#define __MAE_SMV_COMPOSED_MODEL_3D_HPP

#include <vector>
using std::vector;

#include "model_3d.hpp"
using mae::smv::Model3D;

namespace mae
{
	namespace smv
	{
		class ComposedModel3D;
	}
}

/**
A composition of 3D models, superimposed by given offsets.
*/
class mae::smv::ComposedModel3D
{
	/*
	Nested Class Section
	*/

	/** A part of the composed 3D model. */
	class ComposingModel3D: public Model3D
	{
		/** The position of this model within the composed model. */
		private: double offset;

		/**
		Constructs a new composing 3D model with the given offset.

		@param offset This model's offset within the composed model.
		*/
		public: ComposingModel3D();

		/**
		Returns the contribution of this model to the composed model's solid volume.

		@return The contribution of this model to the composed model's solid volume.
		*/
		public: double getComposingSolidVolume();

		/**
		Returns the contribution of this model to the composed model's stereo volume.

		@return The contribution of this model to the composed model's stereo volume.
		*/
		public: double getComposingStereoVolume();

		/**
		Sets the position of this model within the composed model.

		@param offset The position of this model within the composed model.
		*/
		public: void setOffset(double offset);
	};

	/*
	Attribute Section
	*/

	/** The 3D models from which this model is composed. */
	private: vector<ComposingModel3D> components;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	public: ComposedModel3D();

	/*
	Method Section
	*/

	/**
	Adds a new, empty 3D model to the composition, on the given Offset. Returns the 3D model for further processing.

	@param offset The new model's offset.

	@return The new, empty 3D model.
	*/
	public: Model3D& newModel3D(double offset);
	
	public: void clear();

	/*
	Property Section
	*/
	
	public: Model3D& getModel3D(unsigned int index);
	
	public: double getLength();
	
	public: double getHeight();
	
	public: double getWidth();

	/**
	Returns this composed model's solid volume.

	@return This composed model's solid volume.
	*/
	public: double getSolidVolume();

	/**
	Returns this composed model's stereo volume.

	@return This composed model's stereo volume.
	*/
	public: double getStereoVolume();
};

#endif
