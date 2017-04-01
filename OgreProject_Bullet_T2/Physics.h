#ifndef _VIRUS_PHYSICS_
#define _VIRUS_PHYSICS_


#include "OgreBulletDynamicsRigidBody.h" // for OgreBullet
#include "Shapes/OgreBulletCollisionsStaticPlaneShape.h" // for static planes
#include "Shapes/OgreBulletCollisionsBoxShape.h" // for boxes
#include "BaseApplication.h"
#include <iostream>
#include <fstream>

#include <map>
#include <set>

namespace ViRus
{

	//Types

	//Teams
	enum class TeamType : char
	{
		START=0,
		HERO,
		ENEMY,
		END
	};

	//Class declarations
	class Hittable;//Object that can be hit
	class HitObstacle;//Hittable of a static obstacle
	class Teamable;//Object that has a team
	class HitProjectile;//Projectile that can hit objects
	class HitCharacter;//Character that can be hit
	class HitCharAttack;//Character that hits others upon contact
	class HitMap;//Map that goes from the physics object to the hittable object

	//Class definitions

	//Object that can be hit
	class Hittable
	{
		public:
			//Pointer to the scene manager, needed to clean up hittables. Needs to be set to the current scene manager before using HitMap
			static Ogre::SceneManager *ptr_scn_mgr;

		protected:
				//Pointers to clean up at deletion, callback
				OgreBulletDynamics::RigidBody *body;
				OgreBulletCollisions::CollisionShape *shape;
				Ogre::SceneNode *scene;
				void(*callback) (Hittable *) = nullptr;


			public:
				//Complete constructor
				Hittable(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
				:body(ibody),shape(ishape),scene(iscene)
				{}

				//Destructor, calls callback if set and performs clean up
				virtual ~Hittable();

			public:

				//Hit another hittable
				virtual void hit(Hittable &h)
				{}

				//Return true if this hittable is done and can be removed from the map
				virtual bool finished()
				{
					return false;
				}

				//Set the callback to be called before deletion
				void set_callback(void(*icallback) (Hittable *))
				{
					callback = icallback;
				}
	};


	//Hittable of a static obstacle
	class HitObstacle : public Hittable
	{
		public:

			//Complete constructor
			HitObstacle(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene)
			:Hittable(ibody, ishape, iscene)
			{}

			//Destructor
			~HitObstacle() {}
	};

	//Object that has a team
	class Teamable
	{
		protected:
			TeamType team;//Team of this object

		protected:

			//Complete constructor
			Teamable(TeamType iteam)
			:team(iteam)
			{}

		public:

			//Destructor
			virtual ~Teamable() {}

		public:

			//Get this team
			virtual TeamType get_team() const
			{
				return team;
			}

			//Check if this team attacks the other
			virtual bool attack_team(const Teamable &iteam) const;

	};

	//Projectile
	class HitProjectile : public Hittable, public Teamable
	{

		private:
			bool isFinished = false;//Bullet has hit something, and despawns
			int dmg;//Damage that this bullet deals upon hitting a character

		public:

			//Complete constructor
			HitProjectile(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int idmg=0)
			:Hittable(ibody,ishape,iscene), Teamable(iteam), dmg(idmg)
			{}

			//Virtual destructor
			virtual ~HitProjectile() {}

		public:

			//Hit another hittable
			virtual void hit(Hittable &h);

			//Return true if this hittable is done and can be removed from the map
			virtual bool finished()
			{
				return isFinished;
			}
	};

	//Character that can be hit
	class HitCharacter : public Hittable, public Teamable
	{
		protected:
			int health;//Health that his character has (below 0, character is dead)

		public:

			//Complete destructor
			HitCharacter(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int ihealth)
			:Hittable(ibody,ishape,iscene),Teamable(iteam), health(ihealth)
			{}

			virtual ~HitCharacter() {}

		public:

			//Return true if this hittable is done and can be removed from the map
			virtual bool finished()
			{
				return health <= 0;
			}

			//Take damage
			virtual void takeDamage(int idmg)
			{
				health -= idmg;
			}
	};

	//Character that hits others upon contact
	class HitCharAttack : public HitCharacter
	{
		protected:

			int dmg;//Damage that the enemy performs at contact

		public:

			//Complete constructor
			HitCharAttack(OgreBulletDynamics::RigidBody *ibody, OgreBulletCollisions::CollisionShape *ishape, Ogre::SceneNode *iscene, TeamType iteam, int ihealth, int idmg)
			:HitCharacter(ibody, ishape, iscene, iteam, ihealth), dmg(idmg)
			{}

			//Virtual destructor
			virtual ~HitCharAttack() {}

		public:

			//Hit another hittable
			virtual void hit(Hittable &h);


	};


	//Map that goes from the physics object to the hittable object
	class HitMap
	{
		private:

			std::map<btCollisionObject*, Hittable *> hittables;//Map that goes from the bullet object pointer to the hittable pointer
			std::set<btCollisionObject *> clean_up;//Queue of objects to delete

		public:

			~HitMap();

		public:

			//Add a hittable
			void add_hittable(btCollisionObject &c, Hittable &h);

			//Handle collision
			void handle_collision(btCollisionObject *a, btCollisionObject *b);

			//Delete pointers queued for destroctions
			void clean_queued();

		public:

			//Delete all hittables
			void clear_all();
	};

}

#endif // !_VIRUS_PHYSICS_
