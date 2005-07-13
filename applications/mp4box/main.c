/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / mp4box application
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


#include <gpac/scene_manager.h>
#include <gpac/media_tools.h>
/*RTP packetizer flags*/
#include <gpac/ietf.h>
#include <gpac/ismacryp.h>

#define BUFFSIZE	8192

/*in fileimport.c*/
#ifndef GPAC_READ_ONLY
void convert_file_info(char *inName, u32 trackID);
GF_Err import_file(GF_ISOFile *dest, char *inName, u32 import_flags, Double force_fps, u32 frames_per_sample);
GF_Err split_isomedia_file(GF_ISOFile *mp4, Double split_dur, u32 split_size_kb, char *inName, Double InterleavingTime, Double chunk_start);
GF_Err cat_isomedia_file(GF_ISOFile *mp4, char *fileName, u32 import_flags, Double force_fps, u32 frames_per_sample, char *tmp_dir);

GF_Err EncodeFile(char *in, GF_ISOFile *mp4, char *logFile, char *mediaSource, u32 flags, u32 rap_freq);
GF_Err EncodeFileChunk(char *chunkFile, char *bifs, char *inputContext, char *outputContext);
#endif

/*in filedump.c*/
void dump_file_text(char *file, char *inName, u32 dump_mode);
void dump_scene_stats(char *file, char *inName, u32 stat_level);
void PrintNode(const char *name, Bool x3d_node);
void PrintBuiltInNodes(Bool x3d_node);
void dump_file_mp4(GF_ISOFile *file, char *inName);
void dump_file_rtp(GF_ISOFile *file, char *inName);
void dump_file_ts(GF_ISOFile *file, char *inName);
void dump_file_ismacryp(GF_ISOFile *file, char *inName);
void dump_timed_text_track(GF_ISOFile *file, u32 trackID, char *inName, Bool is_convert, Bool to_srt);
void DumpSDP(GF_ISOFile *file, char *inName);
void DumpTrackInfo(GF_ISOFile *file, u32 trackID, Bool full_dump);
void DumpMovieInfo(GF_ISOFile *file);


/*some global vars for swf import :(*/
u32 swf_flags = 0;
Float swf_flatten_angle = 0;


void PrintVersion()
{
	fprintf(stdout, "MP4Box - GPAC version " GPAC_VERSION "\n"
		"GPAC Copyright: (c) Jean Le Feuvre 2000-2005\n\t\t(c) ENST 2005-200X\n");
}

void PrintGeneralUsage()
{
	fprintf(stdout, "General Options:\n"
			" -inter time_in_ms    interleaves file data (track chunks of time_in_ms)\n"
			"                       * Note 1: Interleaving is 0.5s by default\n"
			"                       * Note 2: a value of 0 disables interleaving\n"
			" -flat                stores file with all media data first, non-interleaved\n"
			" -frag time_in_ms     fragments file (track fragments of time_in_ms)\n"
			"                       * Note: Always disables interleaving\n"
			" -out filename        specifies output file name\n"
			"                       * Note: By default input (MP4,3GP) file is overwritten\n"
			" -tmp dirname         specifies directory for temporary file creation\n"
			"                       * Note: Default temp dir is OS-dependent\n"
			" -nosys               removes all MPEG-4 Systems info except IOD (profiles)\n"
			"                       * Note: Set by default whith '-add' and '-cat'\n"
			" -isma                rewrites the file as an ISMA 1.0 AV file\n"
			" -ismax               same as \'-isma\' and removes all clock references\n"
			" -3gp                 rewrites as 3GPP(2) file (no more MPEG-4 Systems Info)\n"
			"                       * Note 1: some tracks may be removed in the process\n"
			"                       * Note 2: always on for *.3gp *.3g2 *.3gpp\n"
			" -brand ABCD          sets major brand of file\n"
			" -ab ABCD             adds given brand to file's alternate brand list\n"
			" -rb ABCD             removes given brand from file's alternate brand list\n"
			" -cprt string         adds copyright string to movie\n"
			" -chap file           adds chapter information contained in file\n"
			" -rem trackID:        removes track from file\n"
			" -new:                forces creation of a new destination file\n"
			" -rem trackID:        removes track from file\n"
			" -lang [tkID=]LAN:    sets track language. LAN is the ISO 639-2 code (eng, und)\n"
			" -delay tkID=TIME:    sets track start delay in ms.\n"
			" -split time_sec      splits in files of time_sec max duration\n"
			"                       * Note: this removes all MPEG-4 Systems media\n"
			" -splits filesize     splits in files of max filesize kB.\n"
			"                       * Note: this removes all MPEG-4 Systems media\n"
			" -splitx start:end    extracts a new file from start to end (in seconds)\n"
			"                       * Note: this removes all MPEG-4 Systems media\n"
			"\n");
}
void PrintFormats()
{
	fprintf(stdout, "Suppported raw formats and file extensions:\n"
			" NHNT                 .media .nhnt .info\n"
			" MPEG Audio           .mp3\n"
			" ADTS-AAC             .aac\n"
			" MPEG-4 Visual        .cmp .m4v\n"
			" H263 Video           .263 .h263\n"
			" AVC/H264 Video       .h264 .h26L .264 .26L\n"
			" JPEG Images          .jpg .jpeg\n"
			" PNG Images           .png\n"
			" AMR(WB) Audio        .amr .awb\n"
			" EVRC Audio           .evc\n"
			" SMV Audio            .smv\n"
			"\n"
			"Supported containers and file extensions:\n"
			" AVI                  .avi\n"
			" MPEG-PS              .mpg .mpeg .vob .vcd .svcd\n"
			" QCP                  .qcp\n"
			" OGG                  .ogg\n"
			" ISO-Media files      no extension checking\n"
			"\n"
			"Supported text formats:\n"
			" SRT Subtitles        .srt\n"
			" SUB Subtitles        .sub\n"
			" GPAC Timed Text      .ttxt\n"
			" QuickTime TeXML Text  .xml  (cf QT documentation)\n"
			"\n"
			"Supported Scene formats:\n"
			" MPEG-4 XMT-A         .xmt .xmta .xmt.gz .xmta.gz\n"
			" MPEG-4 BT            .bt .bt.gz\n"
			" VRML                 .wrl .wrl.gz\n"
			" X3D-XML              .x3d .x3d.gz\n"
			" X3D-VRML             .x3dv .x3dv.gz\n"
			" MacroMedia Flash     .swf (very limitted import support only)\n"
			"\n"
		);
}

void PrintImportUsage()
{
	fprintf(stdout, "Importing Options\n"
			"\nFile importing syntax:\n"
			" \"#video\" \"#audio\":   base import for most AV files\n"
			" \"#trackID=ID\":       track import for IsoMedia and other files\n"
			" \"%%X\":                imports only the first X seconds\n"
			" \";lang=LAN\":         sets imported media language code\n"
			" \";delay=delay_ms\":   sets imported media initial delay in ms\n"
			"\n"
			" -add file:           add file tracks to (new) output file\n"
			" -cat file:           concatenates file samples to (new) output file\n"
			"                       * Note: creates tracks if needed\n"
			"\n"
			" -dref:               keeps media data in original file\n"
			" -nodrop:             forces constant FPS when importing AVI video\n"
			" -packed:             forces packed bitstream when importing raw ASP\n"
			" -sbr:                backward compatible signaling of AAC-SBR\n"
			" -sbrx:               non-backward compatible signaling of AAC-SBR\n"
			"                       * Note: SBR AAC cannot be detected at import time\n"
			" -fps FPS:            forces frame rate for video and SUB subtitles import\n"
			"                       * For raw H263 import, default FPS is 15\n"
			"                       * For all other imports, default FPS is 25\n"
			" -mpeg4:              forces MPEG-4 sample descriptions when possible (3GPP2)\n"
			" -agg N:              aggregates N audio frames in 1 sample (3GP media only)\n"
			"                       * Note: Maximum value is 15 - Disabled by default\n"
			" -keepsys:            keeps all MPEG-4 Systems info when using '-add' / 'cat'\n"
			"\n"
			);
}

void PrintEncodeUsage()
{
	fprintf(stdout, "MPEG-4 Scene Encoding Options\n"
			" -mp4:                specify input file is for encoding.\n"
			" -def:                encode DEF names\n"
			" -sync time_in_ms:    forces BIFS sync sample generation every time_in_ms\n"
			"                       * Note: cannot be used with -shadow\n"
			" -shadow time_ms:     forces BIFS sync shadow sample generation every time_ms.\n"
			"                       * Note: cannot be used with -sync\n"
			" -log:                generates BIFS encoder log file\n"
			" -ms file:            specifies file for track importing\n"
			"\nChunk Processing\n"
			" -inctx file:         specifies initial context (MP4/BT/XMT)\n"
			"                       * Note: input file must be a commands-only file\n"
			" -outctx:             specifies storage of updated context (MP4/BT/XMT)\n"
			"\n"
			);
}

void PrintEncryptUsage()
{
	fprintf(stdout, "ISMA Encryption/Decryption Options\n"
			" -crypt drm_file:     crypts a specific track using ISMA AES CTR 128\n"
			" -decrypt [drm_file]  decrypts a specific track using ISMA AES CTR 128\n"
			"                       * Note: drm_file can be omitted if keys are in file\n"
			" -set-kms kms_uri     changes KMS location for all tracks or a given one.\n"
			"                       * to adress a track, use \'tkID=kms_uri\'\n"
			"\n"
			"DRM file syntax for GPAC ISMACryp:\n"
			"                      File is XML and shall start with xml header\n"
			"                      File is a list of \"ISMACrypTrack\" elements (all at root)\n"
			"\n"
			"ISMACrypTrack attributes are\n"
			" TrackID:             ID of track to en/decrypt\n"
			" key:                 AES-128 key formatted (hex string \'0x\'+32 chars)\n"
			" salt:                CTR IV salt key (64 bits) (hex string \'0x\'+16 chars)\n"
			"\nEncryption only attributes:\n"
			" Scheme_URI:          URI of scheme used\n"
			" KMS_URI:             URI of key management system\n"
			"                       * Note: \'self\' writes key and salt in the file\n"
			" selectiveType:       selective encryption type - understood values are:\n"
			"   \"None\":            all samples encrypted (default)\n"
			"   \"RAP\":             only encrypts random access units\n"
			"   \"Non-RAP\":         only encrypts non-random access units\n"
			"   \"Rand\":            random selection is performed\n"
			"   \"X\":               Encrypts every first sample out of X (uint)\n"
			"   \"RandX\":           Encrypts one random sample out of X (uint)\n"
			"\n"
			" ipmpType:            IPMP Signaling Type: None, IPMP, IPMPX\n"
			" ipmpDescriptorID:    IPMP_Descriptor ID to use if IPMP(X) is used\n"
			"                       * If not set MP4Box will generate one for you\n"
			"\n"
		);
}

void PrintHintUsage()
{
	fprintf(stdout, "Hinting Options\n"
			" -hint:               hints the file for RTP/RTSP\n"
			" -mtu size:           specifies MTU size in bytes. Default size is 1500\n"
			" -copy:               copies media data to hint track rather than reference\n"
			"                       * Note: speeds up server but takes much more space\n"
			" -tight:              performs tight interleaving (sample based) of hinted file\n"
			"                       * Note: reduces server disk seek but increases file size\n"
			" -multi [maxptime]:   enables frame concatenation in RTP packets if possible\n"
			"        maxptime:     max packet duration in ms (optional, default 100ms)\n"
			" -rate ck_rate:       specifies rtp rate in Hz when no default for payload\n"
			"                       * Note: default value is 90000 (MPEG rtp rates)\n"
			" -mpeg4:              forces MPEG-4 generic payload whenever possible\n"
			" -latm:               forces MPG4-LATM transport for AAC streams\n"
			" -static:             enables static RTP payload IDs whenever possible\n"
			"                       * By default, dynamic payloads are always used\n"
			"\n"
			"MPEG-4 Generic Payload Options\n"
			" -ocr:                forces all streams to be synchronized\n"
			"                       * Most RTSP servers only support synchronized streams\n"
			" -rap:                signals random access points in RTP packets\n"
			" -ts:                 signals AU Time Stamps in RTP packets\n"
			" -size:               signals AU size in RTP packets\n"
			" -idx:                signals AU sequence numbers in RTP packets\n"
			" -iod:                prevents systems tracks embedding in IOD\n"
			"                       * Note: shouldn't be used with -isma option\n"
			"\n"
			" -sdp_ex string:      adds sdp string to (hint) track (\"-sdp_ex tkID:string\")\n"
			"                      or movie. This will take care of SDP lines ordering\n"
			"                       * WARNING: You cannot add anything to SDP, cf rfc2327.\n"
			" -unhint:             removes all hinting information.\n"
			"\n");
}
void PrintExtractUsage()
{
	fprintf(stdout, "Extracting Options\n"
			" -raw TrackID:        extracts track in raw format when supported\n" 
			" -raws TrackID:       extract each track sample to a file\n" 
			"                       * Note: \"TrackID:N\" extracts Nth sample\n"
			" -nhnt TrackID:       extracts track in nhnt format\n" 
			" -single TrackID:     extracts track to a new mp4 file\n"
			" -avi TrackID:        extracts visual track to an avi file\n"
			" -qcp TrackID:        same as \'-raw\' but defaults to QCP file for EVRC/SMV\n" 
			" -aviraw TK:          extracts AVI track in raw format\n"
			"                      $TK can be one of \"video\" \"audio\" \"audioN\"\n" 
			"\n");
}
void PrintDumpUsage()
{
	fprintf(stdout, "Dumping Options\n"
			" -std:                dumps to stdout instead of file\n"
			" -info [trackID]      prints movie info / track info if trackID specified\n"
			"                       * Note: for non IsoMedia files, gets import options\n" 
			" -bt:                 scene to bt format - removes unknown MPEG4 nodes\n" 
			" -xmt:                scene to XMT-A format - removes unknown MPEG4 nodes\n" 
			" -wrl:                scene VRML format - removes unknown VRML nodes\n" 
			" -x3d:                scene to X3D/XML format - removes unknown X3D nodes\n" 
			" -x3dv:               scene to X3D/VRML format - removes unknown X3D nodes\n" 
			" -diso:               scene IsoMedia file boxes in XML output\n"
			" -drtp:               rtp hint samples structure to XML output\n"
			" -dts:                prints sample timing to text output\n"
			" -sdp:                dumps SDP description of hinted file\n"
			" -dcr:                ISMACryp samples structure to XML output\n"
			"\n"
#ifndef GPAC_READ_ONLY
			" -ttxt:               Converts input subtitle to GPAC TTXT format\n"
#endif
			" -ttxt TrackID:       Dumps Text track to GPAC TTXT format\n"
#ifndef GPAC_READ_ONLY
			" -srt:                Converts input subtitle to SRT format\n"
#endif
			" -srt TrackID:        Dumps Text track to SRT format\n"
			"\n"
			" -stat:               generates node/field statistics for scene\n"
			" -stats:              generates node/field statistics per MPEG-4 Access Unit\n"
			" -statx:              generates node/field statistics for scene after each AU\n"
			"\n");
}

void PrintMetaUsage()
{
	fprintf(stdout, "Meta handling Options\n"
			" -set-meta args:      sets given meta type - syntax: \"ABCD[;tk=ID]\"\n"
			"                       * ABCD: four char meta type (NULL or 0 to remove meta)\n"
			"                       * [;tk=ID]: if not set use root (file) meta\n"
			"                                if ID is 0 use moov meta\n"
			"                                if ID is not 0 use track meta\n"
			" -add-item args:      adds resource to meta\n"
			"                       * syntax: file_path + options (\';\' separated):\n"
			"                        tk=ID: meta adressing (file, moov, track)\n"
			"                        name=str: item name\n"
			"                        mime=mtype: item mime type\n"
			"                        encoding=enctype: item content-encoding type\n"
			"                       * file_path \"this\" or \"self\": item is the file itself\n"
			" -rem-item args:      removes resource from meta - syntax: item_ID[;tk=ID]\n"
			" -set-primary args:   sets item as primary for meta - syntax: item_ID[;tk=ID]\n"
			" -set-xml args:       sets meta XML data\n"
			"                       * syntax: xml_file_path[;tk=ID][;binary]\n"
			" -rem-xml [tk=ID]:    removes meta XML data\n"
			" -dump-xml args:      dumps meta XML to file - syntax file_path[;tk=ID]\n"
			" -dump-item args:     dumps item to file - syntax item_ID[;tk=ID][;path=fileName]\n"
			"\n");
}

void PrintSWFUsage()
{
	fprintf(stdout, 
			"SWF Importer Options\n"
			"\n"
			"MP4Box can import simple Macromedia Flash files (\".SWF\")\n"
			"You can specify a SWF input file with \'-bt\', \'xmt\' and \'-mp4\' options\n"
			"\n"
			" -static:             all SWF defines are placed in first scene replace\n"
			"                       * Note: By default SWF defines are sent when needed\n"
			" -ctrl:               uses a dedicated stream for movie control\n"
			"                       * Note: Forces \'-static\'\n"
			" -notext:             removes all SWF text\n"
			" -nofont:             removes all embedded SWF Fonts (terminal fonts used)\n"
			" -noline:             removes all lines from SWF shapes\n"
			" -nograd:             removes all gradients from swf shapes\n"
			" -quad:               uses quadratic bezier curves instead of cubic ones\n"
			" -xlp:                support for lines transparency and scalability\n"
			" -flatten ang:        complementary angle below which 2 lines are merged\n"
			"                       * Note: angle \'0\' means no flattening\n"
			"\n"
		);
}

void PrintUsage()
{
	fprintf (stdout, "MP4Box [option] input [option]\n"
#ifndef GPAC_READ_ONLY
			" -h general:          general options help\n"
			" -h hint:             hinting options help\n"
			" -h import:           import options help\n"
			" -h encode:           encode options help\n"
			" -h meta:             meta handling options help\n"
#else
			"READ-ONLY VERSION\n"
#endif
			" -h extract:          extraction options help\n"
			" -h dump:             dump options help\n"
			" -h swf:              Flash (SWF) options help\n"
			" -h crypt:            ISMA E&A options help\n"
			" -h format:           supported formats help\n"
			"\n"
			" -nodes:              lists supported MPEG4 nodes\n"
			" -node NodeName:      gets MPEG4 node syntax and QP info\n"
			" -xnodes:             lists supported X3D nodes\n"
			" -xnode NodeName:     gets X3D node syntax\n"
			"\n"
			" -version:            gets build version\n"
			);
}


#ifndef GPAC_READ_ONLY

/*
		MP4 File Hinting
*/

void SetupClockReferences(GF_ISOFile *file)
{
	u32 i, count, ocr_id;
	count = gf_isom_get_track_count(file);
	if (count==1) return;
	ocr_id = 0;
	for (i=0; i<count; i++) {
		if (!gf_isom_is_track_in_root_od(file, i+1)) continue;
		ocr_id = gf_isom_get_track_id(file, i+1);
		break;
	}
	/*doesn't look like MP4*/
	if (!ocr_id) return;
	for (i=0; i<count; i++) {
		GF_ESD *esd = gf_isom_get_esd(file, i+1, 1);
		if (esd) {
			esd->OCRESID = ocr_id;
			gf_isom_change_mpeg4_description(file, i+1, 1, esd);
			gf_odf_desc_del((GF_Descriptor *) esd);
		}
	}
}

/*base RTP payload type used (you can specify your own types if needed)*/
#define BASE_PAYT		96

GF_Err HintFile(GF_ISOFile *file, u32 MTUSize, u32 max_ptime, u32 rtp_rate, u32 base_flags, Bool copy_data, Bool interleave, Bool regular_iod)
{
	GF_ESD *esd;
	GF_InitialObjectDescriptor *iod;
	u32 i, val, res, streamType;
	u32 sl_mode, prev_ocr, single_ocr, nb_done, tot_bw, bw, flags, spec_type;
	GF_Err e;
	char szType[5], szSubType[5], szPayload[30];
	GF_RTPHinter *hinter;
	Bool copy, has_iod, single_av;
	u8 init_payt = BASE_PAYT;
	u32 iod_mode, mtype;
	u32 media_group = 0;
	u8 media_prio = 0;

	tot_bw = 0;
	prev_ocr = 0;
	single_ocr = 1;
	
	has_iod = 1;
	iod = (GF_InitialObjectDescriptor *) gf_isom_get_root_od(file);
	if (!iod) has_iod = 0;
	else {
		if (!gf_list_count(iod->ESDescriptors)) has_iod = 0;
		gf_odf_desc_del((GF_Descriptor *) iod);
	}

	spec_type = gf_isom_guess_specification(file);
	single_av = gf_isom_is_single_av(file);

	/*first make sure we use a systems track as base OCR*/
	for (i=0; i<gf_isom_get_track_count(file); i++) {
		res = gf_isom_get_media_type(file, i+1);
		if ((res==GF_ISOM_MEDIA_BIFS) || (res==GF_ISOM_MEDIA_OD)) {
			if (gf_isom_is_track_in_root_od(file, i+1)) {
				gf_isom_set_default_sync_track(file, i+1);
				break;
			}
		}
	}

	nb_done = 0;
	for (i=0; i<gf_isom_get_track_count(file); i++) {
		sl_mode = base_flags;
		copy = copy_data;
		/*skip emty tracks (mainly MPEG-4 interaction streams...*/
		if (!gf_isom_get_sample_count(file, i+1)) continue;

		mtype = gf_isom_get_media_type(file, i+1);
		gf_4cc_to_str(mtype, szType);
		switch (mtype) {
		case GF_ISOM_MEDIA_VISUAL:
			if (single_av) {
				media_group = 2;
				media_prio = 2;
			}
			break;
		case GF_ISOM_MEDIA_AUDIO:
			if (single_av) {
				media_group = 2;
				media_prio = 1;
			}
			break;
		case GF_ISOM_MEDIA_HINT:
			continue;
		default:
			/*no hinting of systems track on isma*/
			if (spec_type==FOUR_CHAR_INT('I','S','M','A')) continue;
		}
		mtype = gf_isom_get_media_subtype(file, i+1, 1);
		if ((mtype==GF_ISOM_SUBTYPE_MPEG4) || (mtype==GF_ISOM_SUBTYPE_MPEG4_CRYP) ) mtype = gf_isom_get_mpeg4_subtype(file, i+1, 1);
		gf_4cc_to_str(mtype, szSubType);

		if (!single_av) {
			/*one media per group only (we should prompt user for group selection)*/
			media_group ++;
			media_prio = 1;
		}

		streamType = 0;
		esd = gf_isom_get_esd(file, i+1, 1);
		if (esd) {
			streamType = esd->decoderConfig->streamType;
			if (!prev_ocr) {
				prev_ocr = esd->OCRESID;
				if (!esd->OCRESID) prev_ocr = esd->ESID;
			} else if (esd->OCRESID && prev_ocr != esd->OCRESID) {
				single_ocr = 0;
			}
			/*OD MUST BE WITHOUT REFERENCES*/
			if (streamType==1) copy = 1;
		}
		gf_odf_desc_del((GF_Descriptor *) esd);

		if (!regular_iod && gf_isom_is_track_in_root_od(file, i+1)) {
			/*single AU - check if base64 would fit in ESD (consider 33% overhead of base64), otherwise stream*/
			if (gf_isom_get_sample_count(file, i+1)==1) {
				GF_ISOSample *samp = gf_isom_get_sample(file, i+1, 1, &val);
				if (streamType) {
					res = gf_hinter_can_embbed_data(samp->data, samp->dataLength, streamType);
				} else {
					/*not a system track, we shall hint it*/
					res = 0;
				}
				if (samp) gf_isom_sample_del(&samp);
				if (res) continue;
			}
		}
		if (interleave) sl_mode |= GP_RTP_PCK_USE_INTERLEAVING;

		hinter = gf_hinter_track_new(file, i+1, MTUSize, max_ptime, rtp_rate, sl_mode, init_payt, copy, media_group, media_prio, 
			gf_cbk_on_progress, "Hinting", &e);

		if (!hinter) {
			if (e) {
				fprintf(stdout, "Cannot create hinter (%s)\n", gf_error_to_string(e));
				if (!nb_done) return e;
			}
			continue;
		} 
		bw = gf_hinter_track_get_bandwidth(hinter);
		tot_bw += bw;
		flags = gf_hinter_track_get_flags(hinter);
		gf_hinter_track_get_payload_name(hinter, szPayload);
		fprintf(stdout, "Hinting track ID %d - Type \"%s:%s\" (%s) - BW %d kbps\n", gf_isom_get_track_id(file, i+1), szType, szSubType, szPayload, bw);
/*
		if (flags & GP_RTP_PCK_FORCE_MPEG4) fprintf(stdout, "\tMPEG4 transport forced\n");
		if (flags & GP_RTP_PCK_USE_MULTI) fprintf(stdout, "\tRTP aggregation enabled\n");
*/
		e = gf_hinter_track_process(hinter);

		if (!e) e = gf_hinter_track_finalize(hinter, has_iod);
		gf_hinter_track_del(hinter);
		
		if (e) {
			fprintf(stdout, "Error while hinting (%s)\n", gf_error_to_string(e));
			if (!nb_done) return e;
		}
		init_payt++;
		nb_done ++;
	}

	if (has_iod) {
		iod_mode = GF_SDP_IOD_ISMA;
		if (regular_iod) iod_mode = GF_SDP_IOD_REGULAR;
	} else {
		iod_mode = GF_SDP_IOD_NONE;
	}
	gf_hinter_finalize(file, iod_mode, tot_bw);

	if (!single_ocr)
		fprintf(stdout, "Warning: at least 2 timelines found in the file\nThis may not be supported by servers/players\n\n");

	return GF_OK;
}



static void check_media_profile(GF_ISOFile *file, u32 track)
{
	u8 PL;
	GF_M4ADecSpecInfo dsi;
	GF_ESD *esd = gf_isom_get_esd(file, track, 1);
	if (!esd) return;

	switch (esd->decoderConfig->streamType) {
	case 0x04:
		PL = gf_isom_get_pl_indication(file, GF_ISOM_PL_VISUAL);
		if (esd->decoderConfig->objectTypeIndication==0x20) {
			GF_M4VDecSpecInfo dsi;
			gf_m4v_get_config(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			if (dsi.VideoPL > PL) gf_isom_set_pl_indication(file, GF_ISOM_PL_VISUAL, dsi.VideoPL);
		} else if (esd->decoderConfig->objectTypeIndication==0x21) {
			gf_isom_set_pl_indication(file, GF_ISOM_PL_VISUAL, 0x15);
		} else if (!PL) {
			gf_isom_set_pl_indication(file, GF_ISOM_PL_VISUAL, 0xFE);
		}
		break;
	case 0x05:
		PL = gf_isom_get_pl_indication(file, GF_ISOM_PL_AUDIO);
		switch (esd->decoderConfig->objectTypeIndication) {
		case 0x66: case 0x67: case 0x68: case 0x40:
			gf_m4a_get_config(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &dsi);
			if (dsi.audioPL > PL) gf_isom_set_pl_indication(file, GF_ISOM_PL_AUDIO, dsi.audioPL);
			break;
		default:
			if (!PL) gf_isom_set_pl_indication(file, GF_ISOM_PL_AUDIO, 0xFE);
		}
		break;
	}
	gf_odf_desc_del((GF_Descriptor *) esd);
}

void remove_systems_tracks(GF_ISOFile *file)
{
	u32 i;
	/*force PL rewrite*/
	gf_isom_set_pl_indication(file, GF_ISOM_PL_VISUAL, 0);
	gf_isom_set_pl_indication(file, GF_ISOM_PL_AUDIO, 0);
	gf_isom_set_pl_indication(file, GF_ISOM_PL_OD, 1);	/*the lib always remove IOD when no profiles are specified..*/

	for (i=0; i<gf_isom_get_track_count(file); i++) {
		switch (gf_isom_get_media_type(file, i+1)) {
		case GF_ISOM_MEDIA_VISUAL:
		case GF_ISOM_MEDIA_AUDIO:
		case GF_ISOM_MEDIA_TEXT:
			gf_isom_remove_track_from_root_od(file, i+1);
			check_media_profile(file, i+1);
			break;
		default:
			gf_isom_remove_track(file, i+1);
			i--;
			break;
		}
	}
	/*none required*/
	if (!gf_isom_get_pl_indication(file, GF_ISOM_PL_AUDIO)) gf_isom_set_pl_indication(file, GF_ISOM_PL_AUDIO, 0xFF);
	if (!gf_isom_get_pl_indication(file, GF_ISOM_PL_VISUAL)) gf_isom_set_pl_indication(file, GF_ISOM_PL_VISUAL, 0xFF);

	gf_isom_set_pl_indication(file, GF_ISOM_PL_OD, 0xFF);
	gf_isom_set_pl_indication(file, GF_ISOM_PL_SCENE, 0xFF);
	gf_isom_set_pl_indication(file, GF_ISOM_PL_GRAPHICS, 0xFF);
	gf_isom_set_pl_indication(file, GF_ISOM_PL_INLINE, 0);
}

#endif

/*return value:
	0: not supported 
	1: ISO media 
	2: input bt file (.bt, .wrl)
	3: input XML file (.xmt)
	4: input SWF file (.swf)
*/
u32 get_file_type_by_ext(char *inName)
{
	char *lowername;
	u32 type;
	lowername = strdup(inName);
	if (!lowername) return 0;
	strlwr(lowername);
	if (strstr(lowername, ".mp4") || strstr(lowername, ".3gp") || strstr(lowername, ".mov") || strstr(lowername, ".3g2")) type = 1;
	else if (strstr(lowername, ".bt") || strstr(lowername, ".wrl") || strstr(lowername, ".x3dv")) type = 2;
	else if (strstr(lowername, ".xmt") || strstr(lowername, ".x3d")) type = 3;
	else if (strstr(lowername, ".swf")) type = 4;
	else type = 0;

	/*try open file in read mode*/
	if (!type && gf_isom_probe_file(inName)) type = 1;
	free(lowername);
	return type;
}

static Bool can_convert_to_isma(GF_ISOFile *file)
{
	u32 spec = gf_isom_guess_specification(file);
	if (spec==FOUR_CHAR_INT('I','S','M','A')) return 1;
	return 0;
}



typedef struct
{
	u32 trackID;
	char *line;
} SDPLine;


typedef struct
{
	/*actions:
		0: set meta type
		1: add item
		2: rem item
		3: set item primary
		4: set XML
		5: set binary XML
		6: rem XML
		7: dump item
		8: dump XML
	*/
	u32 act_type;
	Bool root_meta;
	u32 trackID;
	u32 meta_4cc;
	char szPath[GF_MAX_PATH];
	char szName[1024], mime_type[1024], enc_type[1024];
	u32 item_id;
} MetaAction;


/*for SDP_EX, AddTrack and RemTrack*/
#define MAX_CUMUL_OPS	20

static Bool parse_meta_args(MetaAction *meta, char *opts)
{
	Bool ret = 0;
	char szSlot[1024], *next;

	meta->mime_type[0] = 0;
	meta->enc_type[0] = 0;
	meta->szName[0] = 0;
	meta->trackID = 0;
	meta->root_meta = 1;

	if (!opts) return 0;
	while (1) {
		if (!opts || !opts[0]) return ret;
		if (opts[0]==';') opts += 1;
		strcpy(szSlot, opts);
		next = strchr(szSlot, ';');
		if (next) next[0] = 0;
		
		if (!strnicmp(szSlot, "tk=", 3)) {
			sscanf(szSlot, "tk=%d", &meta->trackID);
			meta->root_meta = 0;
			ret = 1;
		}
		else if (!strnicmp(szSlot, "name=", 5)) { strcpy(meta->szName, szSlot+5); ret = 1; }
		else if (!strnicmp(szSlot, "path=", 5)) { strcpy(meta->szPath, szSlot+5); ret = 1; }
		else if (!strnicmp(szSlot, "mime=", 5)) { strcpy(meta->mime_type, szSlot+5); ret = 1; }
		else if (!strnicmp(szSlot, "encoding=", 9)) { strcpy(meta->enc_type, szSlot+9); ret = 1; }
		else if (!stricmp(szSlot, "binary")) {
			if (meta->act_type==4) meta->act_type=5;
			ret = 1;
		}
		else if (!strchr(szSlot, '=')) {
			switch (meta->act_type) {
			case 0:
				if (!stricmp(szSlot, "null") || !stricmp(szSlot, "0")) meta->meta_4cc = 0;
				else meta->meta_4cc = FOUR_CHAR_INT(szSlot[0], szSlot[1], szSlot[2], szSlot[3]);
				ret = 1;
				break;
			case 1: 
			case 4: 
			case 7: 
				strcpy(meta->szPath, szSlot);  
				ret = 1;
				break;
			case 2: 
			case 3: 
			case 8: 
				meta->item_id = atoi(szSlot);  
				ret = 1;
				break;
			}
		}
		opts += strlen(szSlot);
	}
	return ret;
}

#define CHECK_NEXT_ARG	if (i+1==(u32)argc) { fprintf(stdout, "Missing arg - please check usage\n"); return 1; }

#define CHECK_META_OPS	CHECK_NEXT_ARG if (nb_meta_act>=MAX_CUMUL_OPS) { fprintf(stdout, "Sorry - no more than %d meta operations allowed\n", MAX_CUMUL_OPS); return 1; }


typedef struct
{
	/*
	0: rem track
	1: set track language
	2: set track delay
	3: set track KMS URI
	*/
	u32 act_type;
	/*track ID*/
	u32 trackID;
	char lang[4];
	u32 delay_ms;
	const char *kms;
} TrackAction;

int main(int argc, char **argv)
{
	char outfile[5000];
	GF_Err e;
	Double InterleavingTime, split_duration, split_start, import_fps;
	SDPLine sdp_lines[MAX_CUMUL_OPS];
	MetaAction metas[MAX_CUMUL_OPS];
	char *szFilesToCat[MAX_CUMUL_OPS];
	char *szTracksToAdd[MAX_CUMUL_OPS];
	TrackAction tracks[MAX_CUMUL_OPS];
	u32 brand_add[MAX_CUMUL_OPS], brand_rem[MAX_CUMUL_OPS];

	u32 i, MTUSize, stat_level, hint_flags, encode_flags, rap_freq, MakeISMA, Make3GP, info_track_id, import_flags, nb_add, nb_cat, ismaCrypt, agg_samples, nb_sdp_ex, max_ptime, raw_sample_num, split_size, nb_meta_act, nb_track_act, rtp_rate, major_brand, nb_alt_brand_add, nb_alt_brand_rem;
	Bool HintIt, needSave, FullInter, Frag, HintInter, dump_std, dump_rtp, dump_mode, regular_iod, trackID, HintCopy, remove_sys_tracks, remove_hint, force_new, keep_sys_tracks;
	Bool print_sdp, print_info, open_edit, track_dump_type, dump_isom, dump_cr, force_ocr, encode, do_log, do_flat, dump_srt, dump_ttxt, x3d_info, chunk_mode, dump_ts;
	char *inName, *outName, *arg, *mediaSource, *tmpdir, *input_ctx, *output_ctx, *drm_file, *avi2raw, *cprt, *chap_file;
	GF_ISOFile *file;

	if (argc < 2) {
		PrintUsage();
		return 1;
	}

	nb_add = nb_cat = nb_track_act = nb_sdp_ex = max_ptime = raw_sample_num = nb_meta_act = rtp_rate = major_brand = nb_alt_brand_add = nb_alt_brand_rem = 0;
	e = GF_OK;
	split_duration = 0.0f;
	split_start = -1.0f;
	InterleavingTime = 0.5f;
	import_fps = 0;
	import_flags = 0;
	rap_freq = encode_flags = split_size = 0;
	MTUSize = 1500;
	HintCopy = FullInter = HintInter = encode = do_log = 0;
	chunk_mode = dump_mode = Frag = force_ocr = remove_sys_tracks = agg_samples = remove_hint = keep_sys_tracks = 0;
	x3d_info = MakeISMA = Make3GP = HintIt = needSave = print_sdp = print_info = regular_iod = dump_std = open_edit = dump_isom = dump_rtp = dump_cr = dump_srt = dump_ttxt = force_new = dump_ts = 0;
	track_dump_type = 0;
	ismaCrypt = 0;
	file = NULL;
	
	trackID = stat_level = hint_flags = 0;
	info_track_id = 0;
	do_flat = 0;
	inName = outName = mediaSource = input_ctx = output_ctx = drm_file = avi2raw = cprt = chap_file = NULL;

	swf_flags = 0;
	swf_flatten_angle = 0.0f;
	tmpdir = NULL;
	
	/*parse our args*/
	for (i = 1; i < (u32) argc ; i++) {
		arg = argv[i];
		/*main file*/
		if (isalnum(arg[0]) || (arg[0]=='/') || (arg[0]=='.') || (arg[0]=='\\') ) {
			if (inName) { fprintf(stdout, "Error - 2 input names specified, please check usage\n"); return 1; }
			inName = arg;
		}
		else if (!stricmp(arg, "-?")) { PrintUsage(); return 0; }
		else if (!stricmp(arg, "-version")) { PrintVersion(); return 0; }
		else if (!stricmp(arg, "-sdp")) print_sdp = 1;
		else if (!stricmp(arg, "-info")) {
			print_info = 1;
			if ((i+1<(u32) argc) && (sscanf(argv[i+1], "%d", &info_track_id)==1)) {
				char szTk[20];
				sprintf(szTk, "%d", info_track_id);
				if (!strcmp(szTk, argv[i+1])) i++;
				else info_track_id=0;
			} else {
				info_track_id=0;
			}
		}
		else if (!stricmp(arg, "-raw")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_NATIVE;
			trackID = atoi(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-qcp")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_NATIVE | GF_EXPORT_USE_QCP;
			trackID = atoi(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-aviraw")) {
			CHECK_NEXT_ARG
			if (argv[i+1] && !stricmp(argv[i+1], "video")) trackID = 1;
			else if (argv[i+1] && !stricmp(argv[i+1], "audio")) {
				if (strlen(argv[i+1])==5) trackID = 2;
				else trackID = 1 + atoi(argv[i+1] + 5);
			}
			else { fprintf(stdout, "Usage: \"-aviraw video\" or \"-aviraw audio\"\n"); return 1; }
			track_dump_type = GF_EXPORT_AVI_NATIVE;
			i++;
		}
		else if (!stricmp(arg, "-raws")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_RAW_SAMPLES;
			if (strchr(argv[i+1], ':')) {
				sscanf(argv[i+1], "%d:%d", &trackID, &raw_sample_num);
			} else {
				trackID = atoi(argv[i+1]);
			}
			i++;
		}
		else if (!stricmp(arg, "-nhnt")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_NHNT;
			trackID = atoi(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-avi")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_AVI;
			trackID = atoi(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-node")) { CHECK_NEXT_ARG PrintNode(argv[i+1], 0); return (0); }
		else if (!stricmp(arg, "-xnode")) { CHECK_NEXT_ARG PrintNode(argv[i+1], 1); return (0); }
		else if (!stricmp(arg, "-nodes")) { PrintBuiltInNodes(0); return (0); }
		else if (!stricmp(arg, "-xnodes")) { PrintBuiltInNodes(1); return (0); } 
		else if (!stricmp(arg, "-std")) dump_std = 1;
		else if (!stricmp(arg, "-bt")) dump_mode = 1;
		else if (!stricmp(arg, "-xmt")) dump_mode = 2;
		else if (!stricmp(arg, "-wrl")) dump_mode = 3;
		else if (!stricmp(arg, "-x3dv")) dump_mode = 4;
		else if (!stricmp(arg, "-x3d")) dump_mode = 5;
		else if (!stricmp(arg, "-stat")) stat_level = 1;
		else if (!stricmp(arg, "-stats")) stat_level = 2;
		else if (!stricmp(arg, "-statx")) stat_level = 3;
		else if (!stricmp(arg, "-diso")) dump_isom = 1;
		else if (!stricmp(arg, "-dmp4")) {
			dump_isom = 1;
			fprintf(stdout, "WARNING: \"-dmp4\" is deprecated - use \"-diso\" option\n");
		}
		else if (!stricmp(arg, "-drtp")) dump_rtp = 1;
		else if (!stricmp(arg, "-dts")) dump_ts = 1;
		else if (!stricmp(arg, "-dcr")) dump_cr = 1;
		else if (!stricmp(arg, "-ttxt") || !stricmp(arg, "-srt")) {
			if ((i+1<(u32) argc) && (sscanf(argv[i+1], "%d", &trackID)==1)) {
				char szTk[20];
				sprintf(szTk, "%d", trackID);
				if (!strcmp(szTk, argv[i+1])) i++;
				else trackID=0;
			} else {
				trackID = 0;
			}
#ifdef GPAC_READ_ONLY
			if (trackID) { fprintf(stdout, "Error: Read-Only version - subtitle conversion not available\n"); return 1; }
#endif
			if (!stricmp(arg, "-ttxt")) dump_ttxt = 1;
			else dump_srt = 1;
		}

#ifndef GPAC_READ_ONLY
		/*SWF importer options*/
		else if (!stricmp(arg, "-static")) swf_flags |= GF_SM_SWF_STATIC_DICT;
		else if (!stricmp(arg, "-ctrl")) swf_flags |= GF_SM_SWF_SPLIT_TIMELINE;
		else if (!stricmp(arg, "-notext")) swf_flags |= GF_SM_SWF_NO_TEXT;
		else if (!stricmp(arg, "-nofont")) swf_flags |= GF_SM_SWF_NO_FONT;
		else if (!stricmp(arg, "-noline")) swf_flags |= GF_SM_SWF_NO_LINE;
		else if (!stricmp(arg, "-nograd")) swf_flags |= GF_SM_SWF_NO_GRADIENT;
		else if (!stricmp(arg, "-nograd")) swf_flags |= GF_SM_SWF_NO_GRADIENT;
		else if (!stricmp(arg, "-quad")) swf_flags |= GF_SM_SWF_QUAD_CURVE;
		else if (!stricmp(arg, "-xlp")) swf_flags |= GF_SM_SWF_SCALABLE_LINE;
		else if (!stricmp(arg, "-flatten")) {
			CHECK_NEXT_ARG
			swf_flatten_angle = (Float) atof(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-isma")) { MakeISMA = 1; open_edit = 1; }
		else if (!stricmp(arg, "-3gp")) { Make3GP = 1; open_edit = 1; }
		else if (!stricmp(arg, "-nosys")) { remove_sys_tracks = 1; open_edit = 1; }
		else if (!stricmp(arg, "-ismax")) { MakeISMA = 2; open_edit = 1; }
		else if (!stricmp(arg, "-out")) { CHECK_NEXT_ARG outName = argv[i+1]; i++; }
		else if (!stricmp(arg, "-tmp")) { CHECK_NEXT_ARG tmpdir = argv[i+1]; i++; }
		else if (!stricmp(arg, "-cprt")) { CHECK_NEXT_ARG cprt = argv[i+1]; i++; open_edit = 1; }
		else if (!stricmp(arg, "-chap")) { CHECK_NEXT_ARG chap_file = argv[i+1]; i++; open_edit = 1; }
		else if (!stricmp(arg, "-inter")) {
			CHECK_NEXT_ARG
			InterleavingTime = ( (Float) atof(argv[i+1]) ) / 1000;
			open_edit = 1;
			needSave = 1;
			i++;
		} else if (!stricmp(arg, "-frag")) {
			CHECK_NEXT_ARG
			InterleavingTime = ((Float) atof(argv[i+1]) ) / 1000;
			open_edit = 1;
			needSave = 1;
			i++;
			Frag = 1;
		} 
		else if (!stricmp(arg, "-hint")) { open_edit = 1; HintIt = 1; }
		else if (!stricmp(arg, "-unhint")) { open_edit = 1; remove_hint = 1; }
		else if (!stricmp(arg, "-copy")) HintCopy = 1;
		else if (!stricmp(arg, "-tight")) FullInter = 1;
		else if (!stricmp(arg, "-ocr")) force_ocr = 1;
		else if (!stricmp(arg, "-latm")) hint_flags |= GP_RTP_PCK_USE_LATM_AAC;
		else if (!stricmp(arg, "-rap")) hint_flags |= GP_RTP_PCK_SIGNAL_RAP;
		else if (!stricmp(arg, "-ts")) hint_flags |= GP_RTP_PCK_SIGNAL_TS;
		else if (!stricmp(arg, "-size")) hint_flags |= GP_RTP_PCK_SIGNAL_SIZE;
		else if (!stricmp(arg, "-idx")) hint_flags |= GP_RTP_PCK_SIGNAL_AU_IDX;
		else if (!stricmp(arg, "-static")) hint_flags |= GP_RTP_PCK_USE_STATIC_ID;
		else if (!stricmp(arg, "-multi")) {
			hint_flags |= GP_RTP_PCK_USE_MULTI;
			if ((i+1<(u32) argc) && (sscanf(argv[i+1], "%d", &max_ptime)==1)) {
				char szPt[20];
				sprintf(szPt, "%d", max_ptime);
				if (!strcmp(szPt, argv[i+1])) i++;
				else max_ptime=0;
			}
		}
		else if (!stricmp(arg, "-mpeg4")) {
			hint_flags |= GP_RTP_PCK_FORCE_MPEG4;
			import_flags |= GF_IMPORT_FORCE_MPEG4;
		}
		else if (!stricmp(arg, "-mtu")) { CHECK_NEXT_ARG MTUSize = atoi(argv[i+1]); i++; }
		else if (!stricmp(arg, "-rate")) { CHECK_NEXT_ARG rtp_rate = atoi(argv[i+1]); i++; }
		else if (!stricmp(arg, "-sdp_ex")) {
			char *id;
			CHECK_NEXT_ARG
			if (nb_sdp_ex>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d extra SDP lines allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			id = strchr(argv[i+1], ':');
			if (id) {
				id[0] = 0;
				if (sscanf(argv[i+1], "%d", &sdp_lines[0].trackID)==1) {
					id[0] = ':';
					sdp_lines[nb_sdp_ex].line = id+1;
				} else {
					id[0] = ':';
					sdp_lines[nb_sdp_ex].line = argv[i+1];
					sdp_lines[nb_sdp_ex].trackID = 0;
				}
			} else {
				sdp_lines[nb_sdp_ex].line = argv[i+1];
				sdp_lines[nb_sdp_ex].trackID = 0;
			}
			nb_sdp_ex++;
			i++;
		}
		else if (!stricmp(arg, "-single")) {
			CHECK_NEXT_ARG
			track_dump_type = GF_EXPORT_MP4;
			trackID = atoi(argv[i+1]);
			i++;
		}
		else if (!stricmp(arg, "-iod")) regular_iod = 1;
		else if (!stricmp(arg, "-flat")) do_flat = 1;
		else if (!stricmp(arg, "-new")) force_new = 1;
		else if (!stricmp(arg, "-add") || !stricmp(arg, "-import") || !stricmp(arg, "-convert")) {
			CHECK_NEXT_ARG
			if (!stricmp(arg, "-import")) fprintf(stdout, "\tWARNING: \"-import\" is deprecated - use \"-add\"\n");
			else if (!stricmp(arg, "-convert")) fprintf(stdout, "\tWARNING: \"-convert\" is deprecated - use \"-add\"\n");
			if (nb_add>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d add operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			szTracksToAdd[nb_add] = argv[i+1];
			nb_add++;
			i++;
		}
		else if (!stricmp(arg, "-cat")) {
			CHECK_NEXT_ARG
			if (nb_cat>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d cat operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			szFilesToCat[nb_cat] = argv[i+1];
			nb_cat++;
			i++;
		}
		else if (!stricmp(arg, "-rem")) {
			CHECK_NEXT_ARG
			if (nb_track_act>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d track operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			tracks[nb_track_act].act_type = 0;
			tracks[nb_track_act].trackID = atoi(argv[i+1]);
			open_edit = 1;
			nb_track_act++;
			i++;
		}
		else if (!stricmp(arg, "-lang")) {
			char szTK[20], *ext;
			CHECK_NEXT_ARG
			if (nb_track_act>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d track operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			tracks[nb_track_act].act_type = 1;
			tracks[nb_track_act].lang[3] = 0;
			tracks[nb_track_act].trackID = 0;
			strcpy(szTK, argv[i+1]);
			ext = strchr(szTK, '=');
			if (!strnicmp(argv[i+1], "all=", 4)) {
				strncpy(tracks[nb_track_act].lang, argv[i+1]+1, 3);
			} else if (!ext) {
				strncpy(tracks[nb_track_act].lang, argv[i+1], 3);
			} else {
				strncpy(tracks[nb_track_act].lang, ext+1, 3);
				ext[0] = 0;
				tracks[nb_track_act].trackID = atoi(szTK);
			}
			open_edit = 1;
			nb_track_act++;
			i++;
		}
		else if (!stricmp(arg, "-delay")) {
			char szTK[20], *ext;
			CHECK_NEXT_ARG
			if (nb_track_act>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d track operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			strcpy(szTK, argv[i+1]);
			ext = strchr(szTK, '=');
			if (!ext) {
				fprintf(stdout, "Bad format for track delay - expecting ID=LAN got %s\n", argv[i+1]);
				return 1;
			}
			tracks[nb_track_act].act_type = 2;
			tracks[nb_track_act].delay_ms = atoi(ext+1);
			ext[0] = 0;
			tracks[nb_track_act].trackID = atoi(szTK);
			open_edit = 1;
			nb_track_act++;
			i++;
		}
		else if (!stricmp(arg, "-dref")) import_flags |= GF_IMPORT_USE_DATAREF;
		else if (!stricmp(arg, "-nodrop")) import_flags |= GF_IMPORT_NO_FRAME_DROP;
		else if (!stricmp(arg, "-packed")) import_flags |= GF_IMPORT_FORCE_PACKED;
		else if (!stricmp(arg, "-sbr")) import_flags |= GF_IMPORT_SBR_IMPLICIT;
		else if (!stricmp(arg, "-sbrx")) import_flags |= GF_IMPORT_SBR_EXPLICIT;
		else if (!stricmp(arg, "-fps")) { CHECK_NEXT_ARG import_fps = (Float) atof(argv[i+1]); i++; }
		else if (!stricmp(arg, "-agg")) { CHECK_NEXT_ARG agg_samples = atoi(argv[i+1]); i++; }
		else if (!stricmp(arg, "-keepsys")) keep_sys_tracks = 1;
		else if (!stricmp(arg, "-ms")) { CHECK_NEXT_ARG mediaSource = argv[i+1]; i++; }
		else if (!stricmp(arg, "-mp4")) { encode = 1; open_edit = 1; }
		else if (!stricmp(arg, "-log")) do_log = 1; 
		else if (!stricmp(arg, "-def")) encode_flags |= GF_SM_ENCODE_USE_NAMES;
		else if (!stricmp(arg, "-sync")) {
			CHECK_NEXT_ARG
			encode_flags |= GF_SM_ENCODE_RAP_INBAND;
			rap_freq = atoi(argv[i+1]);
			i++;
		} else if (!stricmp(arg, "-shadow")) {
			CHECK_NEXT_ARG
			if (encode_flags & GF_SM_ENCODE_RAP_INBAND) encode_flags ^= GF_SM_ENCODE_RAP_INBAND;
			rap_freq = atoi(argv[i+1]);
			i++;
		} 
		/*chunk encoding*/
		else if (!stricmp(arg, "-outctx")) { CHECK_NEXT_ARG output_ctx = argv[i+1]; i++; }
		else if (!stricmp(arg, "-inctx")) {
			CHECK_NEXT_ARG
			chunk_mode = 1;
			input_ctx = argv[i+1];
			i++;
		}
		else if (!strcmp(arg, "-crypt")) {
			CHECK_NEXT_ARG
			ismaCrypt = 1;
			drm_file = argv[i+1];
			open_edit = 1;
			i += 1;
		}
		else if (!strcmp(arg, "-decrypt")) {
			CHECK_NEXT_ARG
			ismaCrypt = 2;
			if (get_file_type_by_ext(argv[i+1])!=1) {
				drm_file = argv[i+1];
				i += 1;
			}
			open_edit = 1;
		}
		else if (!stricmp(arg, "-set-kms")) {
			char szTK[20], *ext;
			CHECK_NEXT_ARG
			if (nb_track_act>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d track operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			strncpy(szTK, argv[i+1], 19);
			ext = strchr(szTK, '=');
			tracks[nb_track_act].act_type = 3;
			tracks[nb_track_act].trackID = 0;
			if (!strnicmp(argv[i+1], "all=", 4)) {
				tracks[nb_track_act].kms = argv[i+1] + 4;
			} else if (!ext) {
				tracks[nb_track_act].kms = argv[i+1];
			} else {
				tracks[nb_track_act].kms = ext+1;
				ext[0] = 0;
				tracks[nb_track_act].trackID = atoi(szTK);
				ext[0] = '=';
			}
			open_edit = 1;
			nb_track_act++;
			i++;
		}		else if (!stricmp(arg, "-split")) { CHECK_NEXT_ARG split_duration = (Float) atof(argv[i+1]); i++; split_size = 0; }
		else if (!stricmp(arg, "-splits")) { CHECK_NEXT_ARG split_size = atoi(argv[i+1]); i++; split_duration = 0; }
		else if (!stricmp(arg, "-splitx")) { 
			CHECK_NEXT_ARG 
			if (!strstr(argv[i+1], ":")) {
				fprintf(stdout, "Chunk extraction usage: \"-splitx start->end\" expressed in seconds\n");
				return 1;
			}
			sscanf(argv[i+1], "%lf:%lf", &split_start, &split_duration);
			split_duration -= split_start; 
			split_size = 0;
			i++;
		}
		/*meta*/
		else if (!stricmp(arg, "-set-meta")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 0;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-add-item")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 1;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-rem-item")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 2;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-set-primary")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 3;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-set-xml")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 4;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-rem-xml")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 6;
			if (parse_meta_args(&metas[nb_meta_act], argv[i+1])) i++;
			nb_meta_act++;
			open_edit = 1;
		}
		else if (!stricmp(arg, "-dump-xml")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 7;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			i++;
		}
		else if (!stricmp(arg, "-dump-item")) { 
			CHECK_META_OPS
			metas[nb_meta_act].act_type = 8;
			parse_meta_args(&metas[nb_meta_act], argv[i+1]);
			nb_meta_act++;
			i++;
		}
		else if (!stricmp(arg, "-brand")) { 
			char *b = argv[i+1];
			CHECK_NEXT_ARG 
			major_brand = FOUR_CHAR_INT(b[0], b[1], b[2], b[3]);
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-ab")) { 
			char *b = argv[i+1];
			CHECK_NEXT_ARG 
			if (nb_alt_brand_add>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d brand remove operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			brand_add[nb_alt_brand_add] = FOUR_CHAR_INT(b[0], b[1], b[2], b[3]);
			nb_alt_brand_add++;
			open_edit = 1;
			i++;
		}
		else if (!stricmp(arg, "-rb")) { 
			char *b = argv[i+1];
			CHECK_NEXT_ARG 
			if (nb_alt_brand_rem>=MAX_CUMUL_OPS) {
				fprintf(stdout, "Sorry - no more than %d brand remove operations allowed\n", MAX_CUMUL_OPS);
				return 1;
			}
			brand_rem[nb_alt_brand_rem] = FOUR_CHAR_INT(b[0], b[1], b[2], b[3]);
			nb_alt_brand_rem++;
			open_edit = 1;
			i++;
		}
#endif
		else if (!stricmp(arg, "-h")) {
			if (i+1== (u32) argc) PrintUsage();
			else if (!strcmp(argv[i+1], "extract")) PrintExtractUsage();
			else if (!strcmp(argv[i+1], "dump")) PrintDumpUsage();
			else if (!strcmp(argv[i+1], "swf")) PrintSWFUsage();
#ifndef GPAC_READ_ONLY
			else if (!strcmp(argv[i+1], "general")) PrintGeneralUsage();
			else if (!strcmp(argv[i+1], "hint")) PrintHintUsage();
			else if (!strcmp(argv[i+1], "import")) PrintImportUsage();
			else if (!strcmp(argv[i+1], "format")) PrintFormats();
			else if (!strcmp(argv[i+1], "encode")) PrintEncodeUsage();
			else if (!strcmp(argv[i+1], "crypt")) PrintEncryptUsage();
			else if (!strcmp(argv[i+1], "meta")) PrintMetaUsage();
#endif
			else if (!strcmp(argv[i+1], "all")) {
#ifndef GPAC_READ_ONLY
				PrintGeneralUsage();
				PrintHintUsage();
				PrintImportUsage();
				PrintFormats();
				PrintEncodeUsage();
				PrintEncryptUsage();
				PrintMetaUsage();
#endif
				PrintExtractUsage();
				PrintDumpUsage();
				PrintSWFUsage();
			}
			else PrintUsage();
			return 0;
		} else {
			fprintf(stdout, "Option %s unknown. Please check usage\n", arg);
			return 1;
		}
	}
	if (!inName) {
		PrintUsage();
		return 1;
	}

	if ((dump_ttxt || dump_srt) && !trackID) {
		GF_MediaImporter import;
		file = gf_isom_open("ttxt_convert", GF_ISOM_OPEN_WRITE);
		memset(&import, 0, sizeof(GF_MediaImporter));
		import.dest = file;
		import.in_name = inName;
		e = gf_media_import(&import);
		if (e) {
			fprintf(stdout, "Error importing %s: %s\n", inName, gf_error_to_string(e));
			gf_isom_delete(file);
			gf_delete_file("ttxt_convert");
			return 1;
		}
		strcpy(outfile, outName ? outName : inName);
		if (strchr(outfile, '.')) {
			while (outfile[strlen(outfile)-1] != '.') outfile[strlen(outfile)-1] = 0;
			outfile[strlen(outfile)-1] = 0;
		}
		dump_timed_text_track(file, gf_isom_get_track_id(file, 1), dump_std ? NULL : outfile, 1, dump_srt);
		gf_isom_delete(file);
		gf_delete_file("ttxt_convert");
		if (e) {
			fprintf(stdout, "Error converting %s: %s\n", inName, gf_error_to_string(e));
			return 1;
		}
		return 0;
	}
#ifndef GPAC_READ_ONLY
	if (nb_add) {
		u8 open_mode = GF_ISOM_OPEN_EDIT;
		if (force_new) {
			open_mode = (do_flat) ? GF_ISOM_OPEN_WRITE : GF_ISOM_WRITE_EDIT;
		} else {
			FILE *test = fopen(inName, "rb");
			if (!test) {
				open_mode = (do_flat) ? GF_ISOM_OPEN_WRITE : GF_ISOM_WRITE_EDIT;
				if (!outName) outName = inName;
			}
			else fclose(test);
		}

		open_edit = 1;
		file = gf_isom_open(inName, open_mode);
		if (!file) {
			fprintf(stdout, "Cannot open destination file %s: %s\n", inName, gf_error_to_string(gf_isom_last_error(NULL)) );
			return 1;
		}
		if (tmpdir) gf_isom_set_temp_dir(file, tmpdir);
		for (i=0; i<nb_add; i++) {
			e = import_file(file, szTracksToAdd[i], import_flags, import_fps, agg_samples);
			if (e) {
				fprintf(stdout, "Error importing %s: %s\n", szTracksToAdd[i], gf_error_to_string(e));
				gf_isom_delete(file);
				return 1;
			}
		}
		/*unless explicitly asked, remove all systems tracks*/
		if (!keep_sys_tracks) remove_systems_tracks(file);
		needSave = 1;
		if (!MakeISMA && !Make3GP) MakeISMA = can_convert_to_isma(file);
	}

	if (nb_cat) {
		if (!file) {
			u8 open_mode = GF_ISOM_OPEN_EDIT;
			if (force_new) {
				open_mode = (do_flat) ? GF_ISOM_OPEN_WRITE : GF_ISOM_WRITE_EDIT;
			} else {
				FILE *test = fopen(inName, "rb");
				if (!test) {
					open_mode = (do_flat) ? GF_ISOM_OPEN_WRITE : GF_ISOM_WRITE_EDIT;
					if (!outName) outName = inName;
				}
				else fclose(test);
			}

			open_edit = 1;
			file = gf_isom_open(inName, open_mode);
			if (!file) {
				fprintf(stdout, "Cannot open destination file %s: %s\n", inName, gf_error_to_string(gf_isom_last_error(NULL)) );
				return 1;
			}
			if (tmpdir) gf_isom_set_temp_dir(file, tmpdir);
		}
		for (i=0; i<nb_cat; i++) {
			e = cat_isomedia_file(file, szFilesToCat[i], import_flags, import_fps, agg_samples, tmpdir);
			if (e) {
				fprintf(stdout, "Error appending %s: %s\n", szFilesToCat[i], gf_error_to_string(e));
				gf_isom_delete(file);
				return 1;
			}
		}
		/*unless explicitly asked, remove all systems tracks*/
		if (!keep_sys_tracks) remove_systems_tracks(file);

		needSave = 1;
		if (!MakeISMA && !Make3GP) MakeISMA = can_convert_to_isma(file);
	}
#endif
	else if (chunk_mode) {
		if (!inName) {
			fprintf(stdout, "chunk encoding syntax: [-outctx outDump] -inctx inScene auFile\n");
			return 1;
		}
		e = EncodeFileChunk(inName, outName ? outName : inName, input_ctx, output_ctx);
		if (e) fprintf(stdout, "Error encoding chunk file %s\n", gf_error_to_string(e));
		return e ? 1 : 0;
	}
	else if (encode) {
#ifndef GPAC_READ_ONLY
		char logfile[5000];
		if (do_log) {
			strcpy(logfile, inName);
			if (strchr(logfile, '.')) {
				while (logfile[strlen(logfile)-1] != '.') logfile[strlen(logfile)-1] = 0;
				logfile[strlen(logfile)-1] = 0;
			}
			strcat(logfile, "_log.txt");
		}

		strcpy(outfile, outName ? outName : inName);
		if (strchr(outfile, '.')) {
			while (outfile[strlen(outfile)-1] != '.') outfile[strlen(outfile)-1] = 0;
			outfile[strlen(outfile)-1] = 0;
		}
		strcat(outfile, ".mp4");
		file = gf_isom_open(outfile, GF_ISOM_WRITE_EDIT);
		e = EncodeFile(inName, file, do_log ? logfile : NULL, mediaSource ? mediaSource : outfile, encode_flags, rap_freq);
		if (e) goto err_exit;
#else
		fprintf(stdout, "Cannot encode file - read-only release\n");
#endif
		needSave = 1;
	} else if (!file && !(track_dump_type& GF_EXPORT_AVI_NATIVE)) {
		FILE *st = fopen(inName, "rb");
		Bool file_exists = 0;
		if (st) {
			file_exists = 1;
			fclose(st);
		}
		switch (get_file_type_by_ext(inName)) {
		case 1:
			file = gf_isom_open(inName, (u8) (open_edit ? GF_ISOM_OPEN_EDIT : ( (dump_isom>0) ? GF_ISOM_OPEN_READ_DUMP : GF_ISOM_OPEN_READ) ));
			if (!file) {
				if (open_edit) {
					file = gf_isom_open(inName, GF_ISOM_WRITE_EDIT);
					if (!outName && file) outName = inName;
				}

				if (!file) {
					fprintf(stdout, "Error opening file %s: %s\n", inName, gf_error_to_string(gf_isom_last_error(NULL)));
					return 1;
				}
			}
			break;
		/*allowed for bt<->xmt*/
		case 2:
		case 3:
		/*allowed for swf->bt, swf->xmt*/
		case 4:
			break;
		default:
#ifndef GPAC_READ_ONLY
			if (!open_edit && file_exists && !gf_isom_probe_file(inName)) {
				convert_file_info(inName, info_track_id);
				return 0;
			} else
#endif
			if (open_edit) {
				file = gf_isom_open(inName, GF_ISOM_WRITE_EDIT);
				if (!outName && file) outName = inName;
			} else if (!file_exists) {
				fprintf(stdout, "Error creating file %s: %s\n", inName, gf_error_to_string(GF_URL_ERROR));
				return 1;
			} else {
				fprintf(stdout, "Cannot open %s - extension not supported\n", inName);
				return 1;
			}
		}
	}

	strcpy(outfile, outName ? outName : inName);
	if (strchr(outfile, '.')) {
		char szExt[20];
		strcpy(szExt, strchr(outfile, '.'));
		strlwr(szExt);
		/*turn on 3GP saving*/
		if (!strcmp(szExt, ".3gp") || !strcmp(szExt, ".3gpp") || !strcmp(szExt, ".3g2")) {
			MakeISMA = 0;
			Make3GP = 1;
		}

		while (outfile[strlen(outfile)-1] != '.') outfile[strlen(outfile)-1] = 0;
		outfile[strlen(outfile)-1] = 0;
	}

	if (track_dump_type & GF_EXPORT_AVI_NATIVE) {
		char szFile[1024];
		GF_MediaExporter mdump;
		memset(&mdump, 0, sizeof(mdump));
		mdump.in_name = inName;
		mdump.flags = GF_EXPORT_AVI_NATIVE;
		mdump.trackID = trackID;
		if (trackID>2) {
			sprintf(szFile, "%s_audio%d", outfile, trackID-1);
		} else {
			sprintf(szFile, "%s_%s", outfile, (trackID==1) ? "video" : "audio");
		}
		mdump.out_name = szFile;
		e = gf_media_export(&mdump);
		if (e) goto err_exit;
		return 0;
	}

	if (dump_mode) dump_file_text(inName, dump_std ? NULL : outfile, dump_mode-1);
	if (stat_level) dump_scene_stats(inName, dump_std ? NULL : outfile, stat_level);
	if (!HintIt && print_sdp) DumpSDP(file, dump_std ? NULL : outfile);
	if (print_info) {
		if (info_track_id) DumpTrackInfo(file, info_track_id, 1);
		else DumpMovieInfo(file);
	}
	if (dump_isom) dump_file_mp4(file, dump_std ? NULL : outfile);
	if (dump_rtp) dump_file_rtp(file, dump_std ? NULL : outfile);
	if (dump_ts) dump_file_ts(file, dump_std ? NULL : outfile);
	if (dump_cr) dump_file_ismacryp(file, dump_std ? NULL : outfile);
	if ((dump_ttxt || dump_srt) && trackID) dump_timed_text_track(file, trackID, dump_std ? NULL : outfile, 0, dump_srt);
	
	if (split_duration || split_size) {
		split_isomedia_file(file, split_duration, split_size, inName, InterleavingTime, split_start);
	}

	if (track_dump_type) {
		char szFile[1024];
		GF_MediaExporter mdump;
		memset(&mdump, 0, sizeof(mdump));
		mdump.file = file;
		mdump.flags = track_dump_type;
		mdump.trackID = trackID;
		mdump.sample_num = raw_sample_num;
		sprintf(szFile, "%s_track%d", outfile, trackID);
		mdump.out_name = szFile;
		e = gf_media_export(&mdump);
		if (e) goto err_exit;
	}

	for (i=0; i<nb_meta_act; i++) {
		u32 tk = 0;
		u32 item_nb = 0;
		Bool self_ref;
		MetaAction *meta = &metas[i];

		if (meta->trackID) tk = gf_isom_get_track_by_id(file, meta->trackID);
		item_nb = gf_isom_get_meta_item_by_id(file, meta->root_meta, tk, meta->item_id);

		switch (meta->act_type) {
		case 0:
			e = gf_isom_set_meta_type(file, meta->root_meta, tk, meta->meta_4cc);
			needSave = 1;
			break;
		case 1:
			self_ref = !stricmp(meta->szPath, "NULL") || !stricmp(meta->szPath, "this") || !stricmp(meta->szPath, "self");
			e = gf_isom_add_meta_item(file, meta->root_meta, tk, self_ref, self_ref ? NULL : meta->szPath, 
					strlen(meta->szName) ? meta->szName : NULL,  
					strlen(meta->mime_type) ? meta->mime_type : NULL,  
					strlen(meta->enc_type) ? meta->enc_type : NULL,  
					NULL,  NULL);
			needSave = 1;
			break;
		case 2:
			e = gf_isom_remove_meta_item(file, meta->root_meta, tk, item_nb);
			needSave = 1;
			break;
		case 3:
			e = gf_isom_set_meta_primary_item(file, meta->root_meta, tk, item_nb);
			needSave = 1;
			break;
		case 4:
		case 5:
			e = gf_isom_set_meta_xml(file, meta->root_meta, tk, meta->szPath, (meta->act_type==5) ? 1 : 0);
			needSave = 1;
			break;
		case 6:
			e = gf_isom_remove_meta_xml(file, meta->root_meta, tk);
			needSave = 1;
			break;
		case 7:
			e = gf_isom_extract_meta_xml(file, meta->root_meta, tk, meta->szPath, NULL);
			break;
		case 8:
			gf_isom_get_meta_item_info(file, meta->root_meta, tk, item_nb, NULL, NULL, &self_ref, NULL, NULL, NULL, NULL, NULL);
			if (self_ref) {
				e = GF_OK;
				fprintf(stdout, "Warning: item is the file itself - skipping extraction\n");
			} else {
				e = gf_isom_extract_meta_item(file, meta->root_meta, tk, item_nb, strlen(meta->szPath) ? meta->szPath : NULL);
			}
			break;
		}
		if (e) goto err_exit;
	}
	if (!open_edit) {
		if (file) gf_isom_delete(file);
		return 0;
	}

#ifndef GPAC_READ_ONLY


	if (remove_sys_tracks) {
		remove_systems_tracks(file);
		needSave = 1;
		MakeISMA = 0;
	}
	if (remove_hint) {
		for (i=0; i<gf_isom_get_track_count(file); i++) {
			if (gf_isom_get_media_type(file, i+1) == GF_ISOM_MEDIA_HINT) {
				fprintf(stdout, "Removing hint track ID %d\n", gf_isom_get_track_id(file, i+1));
				gf_isom_remove_track(file, i+1);
				i--;
			}
		}
		gf_isom_sdp_clean(file);
		needSave = 1;
	}


	if (!encode) {
		if (outName) {
			strcpy(outfile, outName);
		} else {
			char *rel_name = strrchr(inName, GF_PATH_SEPARATOR);
			if (!rel_name) rel_name = strrchr(inName, '/');
			if (rel_name) sprintf(outfile, "out_%s", rel_name + 1);
			else sprintf(outfile, "out_%s", inName);
		}
		if (MakeISMA) {
			fprintf(stdout, "Converting to ISMA Audio-Video MP4 file...\n");
			/*keep ESIDs when doing ISMACryp*/
			e = gp_media_make_isma(file, ismaCrypt ? 1 : 0, 0, (MakeISMA==2) ? 1 : 0, NULL, NULL);
			if (e) goto err_exit;
			needSave = 1;
		}
		if (Make3GP) {
			fprintf(stdout, "Converting to 3GP file...\n");
			e = gp_media_make_3gpp(file, NULL, NULL);
			if (e) goto err_exit;
			needSave = 1;
		}
		if (ismaCrypt) {
			if (ismaCrypt == 1) {
				if (!drm_file) {
					fprintf(stdout, "Missing DRM file location - usage '-%s drm_file input_file\n", (ismaCrypt==1) ? "crypt" : "decrypt");
					e = GF_BAD_PARAM;
					goto err_exit;
				}
				e = gf_ismacryp_crypt_file(file, drm_file, NULL, NULL);
			} else if (ismaCrypt ==2) {
				e = gf_ismacryp_decrypt_file(file, drm_file, NULL, NULL);
			}
			if (e) goto err_exit;
			needSave = 1;
		}
	}

	for (i=0; i<nb_track_act; i++) {
		TrackAction *tka = &tracks[i];
		u32 track = tka->trackID ? gf_isom_get_track_by_id(file, tka->trackID) : 0;
		u32 timescale = gf_isom_get_timescale(file);
		switch (tka->act_type) {
		case 0:
			e = gf_isom_remove_track(file, track);
			if (e) {
				fprintf(stdout, "Error Removing track ID %d: %s\n", tka->trackID, gf_error_to_string(e));
			} else {
				fprintf(stdout, "Removing track ID %d\n", tka->trackID);
			}
			needSave = 1;
			break;
		case 1:
			for (i=0; i<gf_isom_get_track_count(file); i++) {
				if (track && (track != i+1)) continue;
				e = gf_isom_set_media_language(file, i+1, tka->lang);
				if (e) goto err_exit;
				needSave = 1;
			}
			needSave = 1;
			break;
		case 2:
			if (tka->delay_ms) {
				u32 tk_dur;
				gf_isom_remove_edit_segments(file, track);
				tk_dur = (u32) gf_isom_get_track_duration(file, track);
				gf_isom_append_edit_segment(file, track, (timescale*tka->delay_ms)/1000, 0, GF_ISOM_EDIT_EMPTY);
				gf_isom_append_edit_segment(file, track, tk_dur, 0, GF_ISOM_EDIT_NORMAL);
				needSave = 1;
			} else if (gf_isom_get_edit_segment_count(file, track)) {
				gf_isom_remove_edit_segments(file, track);
				needSave = 1;
			}
			break;
		case 3:
			for (i=0; i<gf_isom_get_track_count(file); i++) {
				if (track && (track != i+1)) continue;
				if (!gf_isom_is_media_encrypted(file, i+1, 1)) continue;
				if (!gf_isom_is_ismacryp_media(file, i+1, 1)) continue;
				e = gf_isom_change_ismacryp_protection(file, i+1, 1, NULL, (char *) tka->kms);
				if (e) goto err_exit;
				needSave = 1;
			}
			break;
		}
		if (e) goto err_exit;
	}


	if (Frag) {
		if (!InterleavingTime) InterleavingTime = 0.5;
		if (HintIt) fprintf(stdout, "Warning: cannot hint and fragment - ignoring hint\n");
		fprintf(stdout, "Fragmenting file (%.3f seconds fragments)\n", InterleavingTime);
		e = gf_media_fragment_file(file, outfile, InterleavingTime, gf_cbk_on_progress, "Fragmenting");
		if (e) fprintf(stdout, "Error while fragmenting file: %s\n", gf_error_to_string(e));
		gf_isom_delete(file);
		if (!e && !outName && !force_new) {
			if (remove(inName)) fprintf(stdout, "Error removing file %s\n", inName);
			else if (rename(outfile, inName)) fprintf(stdout, "Error renaming file %s\n", outfile);
		}
		return (e!=GF_OK) ? 1 : 0;
	}
	
	if (HintIt) {
		if (force_ocr) SetupClockReferences(file);
		fprintf(stdout, "Hinting file with Path-MTU %d Bytes\n", MTUSize);
		MTUSize -= 12;		
		e = HintFile(file, MTUSize, max_ptime, rtp_rate, hint_flags, HintCopy, HintInter, regular_iod);
		if (e) goto err_exit;
		needSave = 1;
		if (print_sdp) DumpSDP(file, dump_std ? NULL : outfile);
	}

	/*full interleave (sample-based) if just hinted*/
	if (HintIt && FullInter) {
		e = gf_isom_set_storage_mode(file, GF_ISOM_STORE_TIGHT);
	} else if (!InterleavingTime) {
		e = gf_isom_set_storage_mode(file, GF_ISOM_STORE_STREAMABLE);
		needSave = 1;
	} else if (do_flat) {
		e = gf_isom_set_storage_mode(file, GF_ISOM_STORE_FLAT);
		needSave = 1;
	} else {
		e = gf_isom_make_interleave(file, InterleavingTime);
	}
	if (e) goto err_exit;

	for (i=0; i<nb_sdp_ex; i++) {
		if (sdp_lines[i].trackID) {
			u32 track = gf_isom_get_track_by_id(file, sdp_lines[i].trackID);
			if (gf_isom_get_media_type(file, track)!=GF_ISOM_MEDIA_HINT) {
				s32 ref_count;
				u32 j, k, count = gf_isom_get_track_count(file);
				for (j=0; j<count; j++) {
					if (gf_isom_get_media_type(file, j+1)!=GF_ISOM_MEDIA_HINT) continue;
					ref_count = gf_isom_get_reference_count(file, j+1, GF_ISOM_REF_HINT);
					if (ref_count<0) continue;
					for (k=0; k<(u32) ref_count; k++) {
						u32 refTk;
						if (gf_isom_get_reference(file, j+1, GF_ISOM_REF_HINT, k+1, &refTk)) continue;
						if (refTk==track) {
							track = j+1;
							j=count;
							break;
						}
					}
				}
			}
			gf_isom_sdp_add_track_line(file, track, sdp_lines[i].line);
			needSave = 1;
		} else {
			gf_isom_sdp_add_line(file, sdp_lines[i].line);
			needSave = 1;
		}
	}

	if (cprt) {
		e = gf_isom_set_copyright(file, "und", cprt);
		needSave = 1;
		if (e) goto err_exit;
	}
	if (chap_file) {
		e = gf_media_import_chapters(file, chap_file, import_fps);
		needSave = 1;
		if (e) goto err_exit;
	}

	if (major_brand) gf_isom_set_brand_info(file, major_brand, 0);
	for (i=0; i<nb_alt_brand_add; i++) gf_isom_modify_alternate_brand(file, brand_add[i], 1);
	for (i=0; i<nb_alt_brand_rem; i++) gf_isom_modify_alternate_brand(file, brand_add[i], 0);

	if (!encode && !force_new) gf_isom_set_final_name(file, outfile);
	if (needSave) {
		if (outName) {
			fprintf(stdout, "Saving to %s: ", outfile);
			gf_isom_set_final_name(file, outfile);
		} else if (encode) {
			fprintf(stdout, "Saving to %s: ", gf_isom_get_filename(file) );
		} else {
			fprintf(stdout, "Saving %s: ", inName);
		}
		if (HintIt && FullInter) fprintf(stdout, "Hinted file - Full Interleaving\n");
		else if (do_flat || !InterleavingTime) fprintf(stdout, "Flat storage\n");
		else fprintf(stdout, "%.3f secs Interleaving\n", InterleavingTime);
		e = gf_isom_close_progress(file, gf_cbk_on_progress, "Writing");
		if (e) goto err_exit;
		if (!outName && !encode && !force_new) {
			if (remove(inName)) fprintf(stdout, "Error removing file %s\n", inName);
			else if (rename(outfile, inName)) fprintf(stdout, "Error renaming file %s\n", outfile);
		}
	} else {
		gf_isom_delete(file);
	}

	if (e) fprintf(stdout, "Error: %s\n", gf_error_to_string(e));
	return (e!=GF_OK) ? 1 : 0;

err_exit:
	if (file) gf_isom_delete(file);
	fprintf(stdout, "\n\tError: %s\n", gf_error_to_string(e));
	return 1;
#else
	gf_isom_delete(file);
	fprintf(stdout, "Error: Read-only version of MP4Box.\n");
	return 1;
#endif
}

