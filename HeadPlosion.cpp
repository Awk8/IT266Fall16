#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"
#include "../client/ClientEffect.h"

#ifndef __GAME_PROJECTILE_H__
#include "../Projectile.h"
#endif

class rvWeaponHeadPlosion : public rvWeapon 
{
public:

	CLASS_PROTOTYPE( rvWeaponHeadPlosion );

	rvWeaponHeadPlosion ( void );
	~rvWeaponHeadPlosion ( void );

	virtual void			Spawn				( void );
	virtual void			Think				( void );

	void					Save( idSaveGame *saveFile ) const;
	void					Restore( idRestoreGame *saveFile );
	void					PreSave				( void );
	void					PostSave			( void );

	#ifdef _XENON
	virtual bool		AllowAutoAim			( void ) const { return false; }
	#endif

	protected:

		virtual void			OnLaunchProjectile	( idProjectile* proj );

		void					SetRocketState		( const char* state, int blendFrames );

		rvClientEntityPtr<rvClientEffect>	guideEffect;
		idList< idEntityPtr<idEntity> >		guideEnts;
		float								guideSpeedSlow;
		float								guideSpeedFast;
		float								guideRange;
		float								guideAccelTime;

		rvStateThread						rocketThread;

		float								reloadRate;

		bool								idleEmpty;

	private:

		stateResult_t		State_Idle				( const stateParms_t& parms );
		stateResult_t		State_Fire				( const stateParms_t& parms );
		stateResult_t		State_Raise				( const stateParms_t& parms );
		stateResult_t		State_Lower				( const stateParms_t& parms );
	
		stateResult_t		State_Rocket_Idle		( const stateParms_t& parms );
		stateResult_t		State_Rocket_Reload		( const stateParms_t& parms );
	
		stateResult_t		Frame_AddToClip			( const stateParms_t& parms );
	
		CLASS_STATES_PROTOTYPE ( rvWeaponHeadPlosion );
	};

CLASS_DECLARATION( rvWeapon, rvWeaponHeadPlosion )
END_CLASS

rvWeaponHeadPlosion::rvWeaponHeadPlosion ( void )
{
}

rvWeaponHeadPlosion::~rvWeaponHeadPlosion ( void ) 
{
	if ( guideEffect ) 
		guideEffect->Stop();
}

void rvWeaponHeadPlosion::Spawn ( void ) {
	float f;

	idleEmpty = false;
	
	spawnArgs.GetFloat ( "lockRange", "0", guideRange );

	spawnArgs.GetFloat ( "lockSlowdown", ".25", f );
	attackDict.GetFloat ( "speed", "0", guideSpeedFast );
	guideSpeedSlow = guideSpeedFast * f;
	
	reloadRate = SEC2MS ( spawnArgs.GetFloat ( "reloadRate", ".8" ) );
	
	guideAccelTime = SEC2MS ( spawnArgs.GetFloat ( "lockAccelTime", ".25" ) );
	
	// Start rocket thread
	rocketThread.SetName ( viewModel->GetName ( ) );
	rocketThread.SetOwner ( this );

	// Adjust reload animations to match the fire rate
	idAnim* anim;
	int		animNum;
	float	rate;
	animNum = viewModel->GetAnimator()->GetAnim ( "reload" );
	if ( animNum ) {
		anim = (idAnim*)viewModel->GetAnimator()->GetAnim ( animNum );
		rate = (float)anim->Length() / (float)SEC2MS(spawnArgs.GetFloat ( "reloadRate", ".8" ));
		anim->SetPlaybackRate ( rate );
	}

	animNum = viewModel->GetAnimator()->GetAnim ( "reload_empty" );
	if ( animNum ) {
		anim = (idAnim*)viewModel->GetAnimator()->GetAnim ( animNum );
		rate = (float)anim->Length() / (float)SEC2MS(spawnArgs.GetFloat ( "reloadRate", ".8" ));
		anim->SetPlaybackRate ( rate );
	}

	SetState ( "Raise", 0 );	
	SetRocketState ( "Rocket_Idle", 0 );
}