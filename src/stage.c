#include "stage.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "main.h"

//Stage constants
typedef signed long fixed_t;

#define FIXED_SHIFT (10)
#define FIXED_UNIT  (1 << FIXED_SHIFT)
#define FIXED_LAND  (FIXED_UNIT - 1)
#define FIXED_UAND  (~FIXED_LAND)

#define FIXED_DEC(d, f) (((d) << FIXED_SHIFT) + (((unsigned long)(f) << FIXED_SHIFT) / 1000000))

#define FIXED_MUL(x, y) (((signed long long)(x) * (y)) >> FIXED_SHIFT)
#define FIXED_DIV(x, y) (((x) << FIXED_SHIFT) / (y))

//Character definitions
#define ASCR_REPEAT 0xFF
#define ASCR_CHGANI 0xFE
#define ASCR_BACK   0xFD

typedef struct
{
	u_char spd;
	const u_char *script;
} CharAnimDef;

typedef struct
{
	u_char tex;
	u_char src[4];
	signed char off[2];
} CharFrame;

typedef struct
{
	u_char texs;
	const char **tex;
	u_char frames;
	const CharFrame *frame;
	const CharAnimDef anim[CharAnim_Max];
} CharDef;

static const CharDef char_defs[CharId_Max] = {
	{ //CharId_Boyfriend
		6,
		(const char*[]){
			"\\BF\\IDLE.TIM;1",
			"\\BF\\LEFT.TIM;1",
			"\\BF\\DOWN.TIM;1",
			"\\BF\\UP.TIM;1",
			"\\BF\\RIGHT.TIM;1",
			"\\BF\\PEACE.TIM;1",
		},
		23,
		(const CharFrame[]){
			{0, {  0,   0, 128, 128}, { 53,  91}}, //0 idle 1
			{0, {128,   0, 128, 128}, { 53,  91}}, //1 idle 2
			{0, {  0, 128, 128, 128}, { 52,  92}}, //2 idle 3
			{0, {128, 128, 128, 128}, { 53,  96}}, //3 idle 4
			
			{1, {  0,   0, 128, 128}, { 54,  93}}, //4 left 1
			{1, {128,   0, 128, 128}, { 56,  94}}, //5 left 2
			{1, {  0, 128, 128, 128}, { 51,  97}}, //6 left miss 1
			{1, {128, 128, 128, 128}, { 52, 101}}, //7 left miss 2
			
			{2, {  0,   0, 128, 128}, { 48,  83}}, //8 down 1
			{2, {128,   0, 128, 128}, { 48,  84}}, //9 down 2
			{2, {  0, 128, 128, 128}, { 49,  90}}, //10 down miss 1
			{2, {128, 128, 128, 128}, { 50,  90}}, //11 down miss 2
			
			{3, {  0,   0, 128, 128}, { 42, 103}}, //12 up 1
			{3, {128,   0, 128, 128}, { 43, 102}}, //13 up 2
			{3, {  0, 128, 128, 128}, { 44,  97}}, //14 up miss 1
			{3, {128, 128, 128, 128}, { 43,  98}}, //15 up miss 2
			
			{4, {  0,   0, 128, 128}, { 42,  94}}, //16 right 1
			{4, {128,   0, 128, 128}, { 42,  94}}, //17 right 2
			{4, {  0, 128, 128, 128}, { 44, 101}}, //18 right miss 1
			{4, {128, 128, 128, 128}, { 43, 101}}, //19 right miss 2
			
			{5, {  0,   0, 128, 128}, { 53,  97}}, //20 peace 1
			{5, {128,   0, 128, 128}, { 53,  96}}, //21 peace 2
			{5, {  0, 128, 128, 128}, { 53,  96}}, //22 peace 3
		},
		{
			{4, (const u_char[]){ 0,  1,  2,  3,  3,  3,  3,  3,  3,  3, ASCR_REPEAT}},                    //CharAnim_Idle
			{2, (const u_char[]){ 4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Left
			{2, (const u_char[]){ 4,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_LeftMiss
			{2, (const u_char[]){ 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Down
			{2, (const u_char[]){ 8, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_DownMiss
			{2, (const u_char[]){12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Up
			{2, (const u_char[]){12, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_UpMiss
			{2, (const u_char[]){16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Right
			{2, (const u_char[]){16, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_RightMiss
			{3, (const u_char[]){20, 21, 22, 22, 22, 22, 22, 22, 22, ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Peace
		}
	},
	{ //CharId_Dad
		5,
		(const char*[]){
			"\\DAD\\IDLE.TIM;1",
			"\\DAD\\LEFT.TIM;1",
			"\\DAD\\DOWN.TIM;1",
			"\\DAD\\UP.TIM;1",
			"\\DAD\\RIGHT.TIM;1",
		},
		19,
		(const CharFrame[]){
			{0, {  0,   0, 128, 128}, { 53,  91}}, //0 idle 1
			{0, {128,   0, 128, 128}, { 53,  91}}, //1 idle 2
			{0, {  0, 128, 128, 128}, { 52,  92}}, //2 idle 3
			{0, {128, 128, 128, 128}, { 53,  96}}, //3 idle 4
			
			{1, {  0,   0, 128, 128}, { 54,  93}}, //4 left 1
			{1, {128,   0, 128, 128}, { 56,  94}}, //5 left 2
			{1, {  0, 128, 128, 128}, { 51,  97}}, //6 left miss 1
			{1, {128, 128, 128, 128}, { 52, 101}}, //7 left miss 2
			
			{2, {  0,   0, 128, 128}, { 48,  83}}, //8 down 1
			{2, {128,   0, 128, 128}, { 48,  84}}, //9 down 2
			{2, {  0, 128, 128, 128}, { 49,  90}}, //10 down miss 1
			{2, {128, 128, 128, 128}, { 50,  90}}, //11 down miss 2
			
			{3, {  0,   0, 128, 128}, { 42, 103}}, //12 up 1
			{3, {128,   0, 128, 128}, { 43, 102}}, //13 up 2
			{3, {  0, 128, 128, 128}, { 44,  97}}, //14 up miss 1
			{3, {128, 128, 128, 128}, { 43,  98}}, //15 up miss 2
			
			{4, {  0,   0, 128, 128}, { 42,  94}}, //16 right 1
			{4, {128,   0, 128, 128}, { 42,  94}}, //17 right 2
			{4, {  0, 128, 128, 128}, { 44, 101}}, //18 right miss 1
			{4, {128, 128, 128, 128}, { 43, 101}}, //19 right miss 2
		},
		{
			{4, (const u_char[]){ 0,  1,  2,  3,  3,  3,  3,  3,  3,  3, ASCR_REPEAT}},                    //CharAnim_Idle
			{2, (const u_char[]){ 4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Left
			{2, (const u_char[]){ 4,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_LeftMiss
			{2, (const u_char[]){ 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Down
			{2, (const u_char[]){ 8, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_DownMiss
			{2, (const u_char[]){12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Up
			{2, (const u_char[]){12, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_UpMiss
			{2, (const u_char[]){16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Right
			{2, (const u_char[]){16, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_RightMiss
			{4, (const u_char[]){ 0,  1,  2,  3,  3,  3,  3,  3,  3,  3, ASCR_REPEAT}},                    //CharAnim_Peace
		}
	},
};

//Stage definitions
typedef struct
{
	CharId id;
	fixed_t x, y;
} StageDef_Char;

typedef struct
{
	//Characters
	StageDef_Char mchar, ochar;
	
	//Song info
	fixed_t bpm;
	fixed_t speed[3];
	
	const char *chart_path;
	const char *music_path;
	int music_channel;
} StageDef;

static const StageDef stage_defs[StageId_Max] = {
	{ //StageId_1_1 (Bopeebo)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(100,0),
		{FIXED_DEC(1,0),FIXED_DEC(1,0),FIXED_DEC(1,600000)},
		"\\CHART\\1.1%c.CHT;1",
		"\\MUSIC\\WEEK1.XA;1", 0,
	},
	{ //StageId_1_2 (Fresh)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(120,0),
		{FIXED_DEC(1,0),FIXED_DEC(1,300000),FIXED_DEC(1,800000)},
		"\\CHART\\1.2%c.CHT;1",
		"\\MUSIC\\WEEK1.XA;1", 2,
	},
	{ //StageId_1_3 (Dadbattle)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(180,0),
		{FIXED_DEC(1,300000),FIXED_DEC(1,500000),FIXED_DEC(2,300000)},
		"\\CHART\\1.3%c.CHT;1",
		"\\MUSIC\\WEEK1.XA;1", 4,
	},
	
	{ //StageId_4_1 (Satin Panties)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(110,0),
		{FIXED_DEC(1,300000),FIXED_DEC(1,600000),FIXED_DEC(1,800000)},
		"\\CHART\\4.1%c.CHT;1",
		"\\MUSIC\\WEEK4.XA;1", 0,
	},
	{ //StageId_4_2 (High)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(125,0),
		{FIXED_DEC(1,300000),FIXED_DEC(1,800000),FIXED_DEC(1,800000)},
		"\\CHART\\4.2%c.CHT;1",
		"\\MUSIC\\WEEK4.XA;1", 2,
	},
	{ //StageId_4_3 (MILF)
		//Characters
		{CharId_Boyfriend,  120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		{CharId_Dad,       -120 << FIXED_SHIFT, 100 << FIXED_SHIFT},
		
		//Song info
		FIXED_DEC(180,0),
		{FIXED_DEC(1,400000),FIXED_DEC(1,700000),FIXED_DEC(2,600000)},
		"\\CHART\\4.3%c.CHT;1",
		"\\MUSIC\\WEEK4.XA;1", 4,
	},
};

//Character state
typedef struct
{
	//Character textures and definition
	const CharAnimDef *anim_def;
	const CharFrame *frame_def;
	CharId char_id;
	
	u_char texs;
	IO_Data *load_tex;
	
	//Character state
	fixed_t x, y;
	
	Gfx_Tex tex;
	u_char tex_i;
	
	CharAnim anim;
	const u_char *anim_p;
	u_char anim_spd, anim_time;
	
	u_char frame;
	u_char load_tex_i;
} Character;

//Stage state
#define SECTION_FLAG_ALTANIM  (1 << 0) //Mom/Dad in Week 5
#define SECTION_FLAG_OPPFOCUS (1 << 1) //Focus on opponent

typedef struct
{
	u_short end;
	u_char flag, pad;
} Section;

#define NOTE_FLAG_OPPONENT    (1 << 2) //Note is opponent's
#define NOTE_FLAG_SUSTAIN     (1 << 3) //Note is a sustain note
#define NOTE_FLAG_SUSTAIN_END (1 << 4) //Draw as sustain ending (this sucks)
#define NOTE_FLAG_MISS        (1 << 6) //Note has been missed
#define NOTE_FLAG_HIT         (1 << 7) //Note has been hit

typedef struct
{
	u_short pos; //steps
	u_char type, pad;
} Note;

typedef struct
{
	//Camera and characters
	struct
	{
		fixed_t x, y, zoom;
		fixed_t tx, ty, td;
	} camera;
	Character character[2];
	
	//HUD textures
	Gfx_Tex tex_note, tex_health;
	
	//Stage data
	const StageDef *stage_def;
	StageId stage_id;
	
	IO_Data chart_data;
	Section *sections;
	Note *notes;
	
	fixed_t speed;
	fixed_t crochet, step_crochet;
	fixed_t early_safe, late_safe;
	fixed_t note_speed;
	
	//Stage state
	Section *cur_section; //Current section
	Note *start_note; //First visible and hittable note, used for drawing and hit detection
	
	fixed_t note_scroll;
	
	u_short song_step;
	u_char just_step, vocal_active;
	
	signed short health;
} Stage;

static Stage stage;

//Stage drawing functions
void Stage_DrawTex(Gfx_Tex *tex, RECT *src, fixed_t x, fixed_t y, fixed_t zoom)
{
	RECT dst = {
		160 + (FIXED_MUL(x, zoom) >> FIXED_SHIFT),
		120 + (FIXED_MUL(y, zoom) >> FIXED_SHIFT),
		FIXED_MUL(src->w, zoom),
		FIXED_MUL(src->h, zoom),
	};
	Gfx_DrawTex(tex, src, &dst);
}

//HUD functions
void HUD_GetNotePos(int i, fixed_t *x, fixed_t *y, u_short pos)
{
	if (x != NULL)
	{
		if (i & 4)
			*x = ((32 - SCREEN_WIDTH2) + (i & 3) * 34) << FIXED_SHIFT; //Opponent
		else
			*x = ((SCREEN_WIDTH2 - 134) + (i & 3) * 34) << FIXED_SHIFT; //BF
	}
	if (y != NULL)
	{
		*y = FIXED_UNIT * -90;
		if (pos != 0xFFFF)
			*y += FIXED_MUL((((fixed_t)pos << FIXED_SHIFT) - stage.note_scroll), stage.note_speed);
	}
}

//Stage music functions
void Stage_StartVocal()
{
	if (!stage.vocal_active)
	{
		Audio_ChannelXA(stage.stage_def->music_channel);
		stage.vocal_active = 1;
	}
}

void Stage_CutVocal()
{
	if (stage.vocal_active)
	{
		Audio_ChannelXA(stage.stage_def->music_channel + 1);
		stage.vocal_active = 0;
	}
}

//Character functions
void Character_SetFrame(Character *this, u_char frame)
{
	//Get frame definition
	const CharFrame *frame_def = &this->frame_def[this->frame = frame];
	
	//Load texture
	if (this->tex_i != frame_def->tex)
		Gfx_LoadTex(&this->tex, this->load_tex[this->tex_i = frame_def->tex], 0);
}

void Character_Animate(Character *this)
{
	//Wait for time
	if (this->anim_time)
	{
		this->anim_time--;
		return;
	}
	
	while (1)
	{
		//Read script
		switch (this->anim_p[0])
		{
			case ASCR_REPEAT:
				this->anim_p = this->anim_def[this->anim].script;
				break;
			case ASCR_CHGANI:
				this->anim_p = this->anim_def[this->anim = this->anim_p[1]].script;
				break;
			case ASCR_BACK:
				this->anim_time = this->anim_def[this->anim].spd;
				this->anim_p -= this->anim_p[1];
				break;
			default:
				if (this->anim_p[0] != this->frame)
					Character_SetFrame(this, this->anim_p[0]);
				this->anim_time = this->anim_def[this->anim].spd;
				this->anim_p++;
				return;
		}
	}
}

void Character_SetAnim(Character *this, CharAnim anim)
{
	//Get animation definition
	const CharAnimDef *anim_def = &this->anim_def[this->anim = anim];
	
	//Start animation
	this->anim_p = anim_def->script;
	this->anim_time = 0;
	this->anim_spd = anim_def->spd;
}

void Character_Load(Character *this, const StageDef_Char *sdef)
{
	//Get character definition
	const CharDef *char_def = &char_defs[this->char_id = sdef->id];
	
	//Use definitions
	this->anim_def = char_def->anim;
	this->frame_def = char_def->frame;
	
	//Load character textures
	u_char texs = this->texs = char_def->texs;
	size_t size;
	IO_Data *texp = this->load_tex = malloc3(size = texs * sizeof(IO_Data));
	if (texp == NULL)
	{
		sprintf(error_msg, "[Character_Load] Failed to allocate texture pointer buffer");
		ErrorLock();
		return;
	}
	
	const char **dtexp = char_def->tex;
	for (; texs != 0; texs--)
		*texp++ = IO_Read(*dtexp++);
	
	//Initialize character state
	this->x = sdef->x;
	this->y = sdef->y;
	this->frame = this->tex_i = 0xFF; //Force frame load
	Character_SetAnim(this, CharAnim_Idle);
}

void Character_Unload(Character *this)
{
	//Unload character textures
	IO_Data *texp = this->load_tex;
	u_char texs = this->texs;
	for (; texs != 0; texs--)
		free3(*texp++);
	free3(this->load_tex);
}

void Character_Draw(Character *this, fixed_t bump)
{
	//Get frame definition
	const CharFrame *frame_def = &this->frame_def[this->frame];
	
	//Get offset
	fixed_t ox = frame_def->off[0] * stage.camera.zoom;
	fixed_t oy = frame_def->off[1] * stage.camera.zoom;
	
	//Draw character
	fixed_t x = this->x - stage.camera.x - ox;
	fixed_t y = this->y - stage.camera.y - oy;
	
	RECT src = {frame_def->src[0], frame_def->src[1], frame_def->src[2], frame_def->src[3]};
	Stage_DrawTex(&this->tex, &src, x, y, FIXED_MUL(stage.camera.zoom, bump));
}

void Character_DrawHealth(Character *this, int ox, fixed_t bump)
{
	int dying;
	if (ox < 0)
		dying = (stage.health >= 19000) * 24;
	else
		dying = (stage.health <= 1000) * 24;
	
	fixed_t hx = (128 << FIXED_SHIFT) * (10000 - stage.health) / 10000;
	RECT src = {(this->char_id % 5) * 48 + dying, 16 + (this->char_id / 5) * 24, 24, 24};
	Stage_DrawTex(&stage.tex_health, &src, hx + ox * (11 << FIXED_SHIFT) - (12 << FIXED_SHIFT), (SCREEN_HEIGHT2 - 32 + 4 - 12) << FIXED_SHIFT, FIXED_MUL(stage.camera.zoom, bump));
}

static const CharAnim note_anims[4][2] = {
	{CharAnim_Left,  CharAnim_LeftMiss},
	{CharAnim_Down,  CharAnim_DownMiss},
	{CharAnim_Up,    CharAnim_UpMiss},
	{CharAnim_Right, CharAnim_RightMiss},
};

void Character_MissNote(Character *this, u_char type)
{
	//Stop playing vocal track
	Stage_CutVocal();
}

void Character_NoteCheck(Character *this, u_char type)
{
	Note *note = stage.start_note;
	for (;; note++)
	{
		//Check if note can be hit
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
			continue;
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_safe < stage.note_scroll)
			continue;
		
		//Hit the note
		Stage_StartVocal();
		Character_SetAnim(this, note_anims[type & 0x3][0]);
		note->type |= NOTE_FLAG_HIT;
		stage.health += 230;
		return;
	}
	
	//Missed
	Character_SetAnim(this, note_anims[type & 0x3][1]);
	Character_MissNote(this, type & 0x3);
	stage.health -= 400;
}

void Character_SustainCheck(Character *this, u_char type)
{
	Note *note = stage.start_note;
	for (;; note++)
	{
		//Check if note can be hit
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
			continue;
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_safe < stage.note_scroll)
			continue;
		
		//Hit the note
		Stage_StartVocal();
		if (this->anim != note_anims[type & 0x3][0])
			Character_SetAnim(this, note_anims[type & 0x3][0]);
		note->type |= NOTE_FLAG_HIT;
		stage.health += 230;
		return;
	}
	
	//Hold animation
	if (stage.just_step && this->anim == note_anims[type & 0x3][0])
		Character_SetAnim(this, note_anims[type & 0x3][0]);
}

//Stage functions
void Stage_FocusChar(Character *ch, fixed_t div)
{
	stage.camera.tx = ch->x * 2 / 3;
	stage.camera.ty = ch->y / 3 - (32 << FIXED_SHIFT);
	stage.camera.td = div;
}

void Stage_Load(StageId id, StageDiff difficulty)
{
	//Get stage definition
	const StageDef *stage_def = stage.stage_def = &stage_defs[stage.stage_id = id];
	
	//Load HUD textures
	Gfx_LoadTex(&stage.tex_note, IO_Read("\\STAGE\\NOTE.TIM;1"), GFX_LOADTEX_FREE);
	Gfx_LoadTex(&stage.tex_health, IO_Read("\\STAGE\\HEALTH.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load characters
	Character_Load(&stage.character[0], &stage_def->mchar);
	Character_Load(&stage.character[1], &stage_def->ochar);
	
	//Load stage data
	char chart_path[64];
	sprintf(chart_path, stage_def->chart_path, "ENH"[difficulty]);
	stage.chart_data = IO_Read(chart_path);
	stage.sections = (Section*)((u_char*)stage.chart_data + 2);
	stage.notes = (Note*)((u_char*)stage.chart_data + 4 + (*((u_short*)stage.chart_data) << 2));
	
	stage.cur_section = stage.sections;
	stage.start_note = stage.notes;
	
	stage.speed = stage_def->speed[difficulty];
	
	stage.crochet = FIXED_DIV(stage_def->bpm, 60 << FIXED_SHIFT);
	stage.step_crochet = stage.crochet * 4;
	
	stage.note_speed = FIXED_MUL(FIXED_DIV(FIXED_DEC(140, 0), stage.step_crochet), stage.speed);
	
	stage.late_safe = FIXED_MUL(FIXED_DEC(0, 166667), stage.step_crochet);
	stage.early_safe = stage.late_safe >> 1;
	
	//Initialize stage state
	stage.note_scroll = -8 << FIXED_SHIFT;
	
	stage.just_step = 0;
	
	stage.health = 10000;
	
	//Initialize camera
	Stage_FocusChar(&stage.character[(stage.cur_section->flag & SECTION_FLAG_OPPFOCUS) != 0], FIXED_UNIT / 24);
	stage.camera.x = stage.camera.tx;
	stage.camera.y = stage.camera.ty;
	stage.camera.zoom = FIXED_UNIT;
}

void Stage_Unload()
{
	//Unload stage data
	free3(stage.chart_data);
	
	//Unload characters
	Character_Unload(&stage.character[0]);
	Character_Unload(&stage.character[1]);
}

void Stage_Tick()
{
	//Get song position
	int playing;
	
	u_short next_step;
	if (stage.note_scroll < 0)
	{
		//Count up scroll
		fixed_t next_scroll = stage.note_scroll + FIXED_MUL(FIXED_DEC(0, 16667), stage.step_crochet); //TODO: PAL
		
		//3 2 1 GO - pre song start
		
		//Update song
		if (next_scroll >= 0)
		{
			//Start song
			Audio_PlayXA(stage.stage_def->music_path, 127, stage.stage_def->music_channel, 0);
			stage.note_scroll = 0;
		}
		else
		{
			//Update scroll
			stage.note_scroll = next_scroll;
		}
		
		next_step = 0;
		playing = 0;
	}
	else if (Audio_PlayingXA())
	{
		//XA position
		fixed_t song_time = (Audio_TellXA_Milli() << FIXED_SHIFT) / 1000;
		
		//Get step position and bump
		stage.note_scroll = FIXED_MUL(song_time, stage.step_crochet);
		
		next_step = stage.note_scroll >> FIXED_SHIFT;
		playing = 1;
	}
	else
	{
		//Song has ended
		stage.note_scroll += FIXED_MUL(FIXED_DEC(0, 16667), stage.step_crochet); //TODO: PAL
		
		next_step = stage.note_scroll >> FIXED_SHIFT;
		playing = 1;
	}
	
	if (next_step > stage.song_step)
		stage.just_step = 1;
	else
		stage.just_step = 0;
	stage.song_step = next_step;
	
	//Get bump
	fixed_t bump;
	if (stage.note_scroll > 0)
	{
		//Bump every 16 steps
		if ((stage.song_step & 0xF) == 0)
			bump = (fixed_t)FIXED_UNIT + ((fixed_t)(FIXED_DEC(0,750000) - (stage.note_scroll & FIXED_LAND)) / 16);
		else
			bump = FIXED_UNIT;
	}
	else
	{
		//Song isn't playing yet
		bump = FIXED_UNIT;
	}
	
	//Update section
	while (1)
	{
		//Check if current section has ended
		if (stage.song_step < stage.cur_section->end)
			break;
		
		//Start next section
		stage.cur_section++;
		Stage_FocusChar(&stage.character[(stage.cur_section->flag & SECTION_FLAG_OPPFOCUS) != 0], FIXED_UNIT / 24);
	}
	
	//Scroll camera
	fixed_t dx = stage.camera.tx - stage.camera.x;
	fixed_t dy = stage.camera.ty - stage.camera.y;
	stage.camera.x += FIXED_MUL(dx, stage.camera.td);
	stage.camera.y += FIXED_MUL(dy, stage.camera.td);
	
	//Handle player note presses
	if (pad_state.press & PAD_LEFT)
		Character_NoteCheck(&stage.character[0], 0);
	if (pad_state.press & PAD_DOWN)
		Character_NoteCheck(&stage.character[0], 1);
	if (pad_state.press & PAD_UP)
		Character_NoteCheck(&stage.character[0], 2);
	if (pad_state.press & PAD_RIGHT)
		Character_NoteCheck(&stage.character[0], 3);
	
	if (pad_state.held & PAD_LEFT)
		Character_SustainCheck(&stage.character[0], 0);
	if (pad_state.held & PAD_DOWN)
		Character_SustainCheck(&stage.character[0], 1);
	if (pad_state.held & PAD_UP)
		Character_SustainCheck(&stage.character[0], 2);
	if (pad_state.held & PAD_RIGHT)
		Character_SustainCheck(&stage.character[0], 3);
	
	//Hardcoded stage animations
	switch (stage.stage_id)
	{
		case StageId_1_1:
			if (stage.just_step && (stage.song_step & 0x1F) == 28)
				Character_SetAnim(&stage.character[0], CharAnim_Peace);
			break;
		default:
			break;
	}
	
	//Process notes
	Note *note;
	
	note = stage.start_note;
	for (;; note++)
	{
		if (note->pos > stage.song_step)
			break;
		
		if (playing && !(note->type & (NOTE_FLAG_HIT | NOTE_FLAG_MISS)))
		{
			//Miss note if out of hit range
			fixed_t note_fp = note->pos << FIXED_SHIFT;
			if (!(note->type & NOTE_FLAG_OPPONENT) && note_fp + stage.late_safe < stage.note_scroll)
			{
				//Missed note
				Character_MissNote(&stage.character[0], note->type & 0x3);
				note->type |= NOTE_FLAG_MISS;
				stage.health -= 475;
			}
			
			//Hit if note is opponent's
			if (note->type & NOTE_FLAG_OPPONENT)
			{
				//Opponent hits note
				Stage_StartVocal();
				Character_SetAnim(&stage.character[1], note_anims[note->type & 0x3][0]);
				note->type |= NOTE_FLAG_HIT;
			}
		}
	}
	
	//Perform health checks
	if (stage.health <= 0)
	{
		sprintf(error_msg, "YOU DIED");
		ErrorLock();
	}
	if (stage.health > 20000)
		stage.health = 20000;
	
	//Draw health heads
	Character_DrawHealth(&stage.character[0],  1, bump);
	Character_DrawHealth(&stage.character[1], -1, bump);
	
	//Draw health bar
	RECT health_back = {0, 8, 256, 8};
	RECT health_fill = {0, 0, 256 - (256 * stage.health / 20000), 8};
	Stage_DrawTex(&stage.tex_health, &health_fill, -128 << FIXED_SHIFT, (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, bump);
	Stage_DrawTex(&stage.tex_health, &health_back, -128 << FIXED_SHIFT, (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, bump);
	
	//Draw notes
	note = stage.start_note;
	for (;; note++)
	{
		//Get note position
		fixed_t x, y;
		HUD_GetNotePos(note->type & 0x7, &x, &y, note->pos);
		
		//Check if went above screen
		if (y < ((-16 - SCREEN_HEIGHT2) << FIXED_SHIFT))
		{
			//Update start note
			stage.start_note++;
		}
		else
		{
			//Don't draw if below screen or already hit
			if (y > ((SCREEN_HEIGHT2 + 16) << FIXED_SHIFT) || note->pos == 0xFFFF)
				break;
			if (note->type & NOTE_FLAG_HIT)
				continue;
			
			//Draw note
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				RECT note_src = {160, ((note->type & 0x3) << 5), 32, 32};
				Stage_DrawTex(&stage.tex_note, &note_src, x - (16 << FIXED_SHIFT), y - (8 << FIXED_SHIFT), bump);
			}
			else
			{
				RECT note_src = {32 + ((note->type & 0x3) << 5), 0, 32, 32};
				Stage_DrawTex(&stage.tex_note, &note_src, x - (16 << FIXED_SHIFT), y - (16 << FIXED_SHIFT), bump);
			}
		}
	}
	
	//Draw note HUD
	RECT note_src = {0, 0, 32, 32};
	
	for (int i = 0; i < 4; i++, note_src.y += 32)
	{
		fixed_t x, y;
		
		//BF
		HUD_GetNotePos(i, &x, &y, 0xFFFF);
		Stage_DrawTex(&stage.tex_note, &note_src, x - (16 << FIXED_SHIFT), y - (16 << FIXED_SHIFT), bump);
		
		//Opponent
		HUD_GetNotePos(4 + i, &x, &y, 0xFFFF);
		Stage_DrawTex(&stage.tex_note, &note_src, x - (16 << FIXED_SHIFT), y - (16 << FIXED_SHIFT), bump);
	}
	
	//Animate and draw characters
	Character_Animate(&stage.character[0]);
	Character_Draw(&stage.character[0], bump);
	
	Character_Animate(&stage.character[1]);
	Character_Draw(&stage.character[1], bump);
}
