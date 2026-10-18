#!/usr/bin/env python3
"""Strict importer tests using synthetic closures and the checked-in local catalog."""
import copy
import json
from pathlib import Path
import sys
import unittest
import xml.etree.ElementTree as ET
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from event_compile import Compiler, Unsupported, SELECTED, MAX_NODES, header, wrapped_rows, read_archive, PkgArchive, build
ROOT=Path(__file__).resolve().parents[1]

def compile_xml(xml, definitions=None, strings=None):
    compiler=Compiler(definitions or {}, strings or {})
    node=compiler.compile(ET.fromstring(xml))
    return compiler,node

class EventCompilerTest(unittest.TestCase):
    def test_resources_and_hull_sign(self):
        c,n=compile_xml('<event><item_modify><item type="scrap" min="-10" max="-6"/><item type="fuel" min="1" max="3"/></item_modify><damage amount="-5"/></event>')
        self.assertEqual(c.effects,[dict(kind=1,min=-10,max=-6),dict(kind=0,min=1,max=3),dict(kind=4,min=5,max=5)])
        self.assertEqual(c.nodes[n]['effect_count'],3)

    def test_full_choices_and_requirement(self):
        c,n=compile_xml('<event><text>start</text><choice req="engi" hidden="true"><text>Repair</text><event><text>done</text><choice><text>Continue</text><event/></choice></event></choice><choice><text>Leave</text><event/></choice></event>')
        node=c.nodes[n];self.assertEqual(node['choice_count'],2)
        choice=c.choices[node['first_choice']]
        self.assertEqual((choice['req_kind'],choice['req_value'],choice['req_level']),(1,1,1))
        self.assertEqual(c.nodes[choice['next']]['choice_count'],1)

    def test_four_choices_preserved(self):
        c,n=compile_xml('<event>'+''.join('<choice><text>Choice%d</text><event/></choice>'%i for i in range(4))+'</event>')
        self.assertEqual(c.nodes[n]['choice_count'],4)

    def test_missing_reference(self):
        with self.assertRaisesRegex(Unsupported,'missing'):
            compile_xml('<event load="MISSING"/>')

    def test_reference_cycles(self):
        e=ET.fromstring('<event name="LOOP"><choice><text>loop</text><event load="LOOP"/></choice></event>')
        with self.assertRaisesRegex(Unsupported,'cyclic'):
            compile_xml('<event load="LOOP"/>',{('event','LOOP'):('fixture',e)})

    def test_list_all_alternatives_kept(self):
        e=ET.fromstring('<eventList name="LIST"><event><text>first</text></event><event><text>second</text><item_modify><item type="fuel" min="2" max="2"/></item_modify></event></eventList>')
        c,n=compile_xml('<event load="LIST"/>',{('eventList','LIST'):('fixture',e)})
        self.assertEqual(c.nodes[n]['alt_count'],2)
        self.assertEqual([c.nodes[x]['text'] for x in c.alternatives],['first','second'])

    def test_unknown_branch_rejects_entire_list(self):
        e=ET.fromstring('<eventList name="LIST"><event><text>safe</text></event><event><ship load="MISSING"/></event></eventList>')
        with self.assertRaisesRegex(Unsupported,'ship'):
            compile_xml('<event load="LIST"/>',{('eventList','LIST'):('fixture',e)})

    def test_text_variants_repeat_complete_effects(self):
        e=ET.fromstring('<textList name="T"><text id="a"/><text id="b" planet="NONE"/></textList>')
        c,n=compile_xml('<event><text load="T"/><item_modify><item type="fuel" min="1" max="1"/></item_modify></event>',{('textList','T'):('fixture',e)},{'a':'First','b':'Second'})
        self.assertEqual(c.nodes[n]['alt_count'],2)
        self.assertEqual([c.nodes[x]['effect_count'] for x in c.alternatives],[1,1])

    def test_filtered_random_alternative_is_rejected(self):
        for extra in ('req="engi"', 'min="2"', 'max="3"', 'weight="2"'):
            e=ET.fromstring('<eventList name="LIST"><event '+extra+'><text>filtered</text></event><event/></eventList>')
            with self.subTest(extra=extra),self.assertRaises(Unsupported):
                compile_xml('<event load="LIST"/>',{('eventList','LIST'):('fixture',e)})

    def test_parent_cannot_exceed_node_limit_after_children(self):
        c=Compiler({}, {})
        c.nodes=[{} for _ in range(MAX_NODES-1)]
        with self.assertRaisesRegex(Unsupported,'node limit'):
            c.compile(ET.fromstring('<event><choice><text>next</text><event/></choice></event>'))
        self.assertEqual(len(c.nodes),MAX_NODES)

    def test_explicit_empty_and_malformed_text_rejected(self):
        for xml in ('<event><text/></event>','<event><text>   </text></event>',
                    '<event><text id="a">ignored</text></event>',
                    '<event><text load="T"><ship/></text></event>',
                    '<event><text load="T">ignored</text></event>',
                    '<event><text><ship/></text></event>',
                    '<event unique="1"/>','<event unique="TRUE"/>'):
            with self.subTest(xml=xml),self.assertRaises(Unsupported):
                compile_xml(xml, strings={'a':'original'})
        with self.assertRaisesRegex(Unsupported,'empty text'):
            compile_xml('<event><text id="a"/></event>',strings={'a':''})
        compile_xml('<event/>')  # Omitted narrative is valid for effect-only terminals.

    def test_choice_wrap_budget_and_immediate_alternative(self):
        label=' '.join(['abcdefghijklmnopqrs']*7)  # V62: seven19char words need7rows; one choice allows6.
        self.assertEqual(wrapped_rows(label),7)
        with self.assertRaisesRegex(Unsupported,'wrapped row'):
            compile_xml('<event><choice><text>'+label+'</text><event/></choice></event>')
        label=' '.join(['abcdefghijklmnopqrs']*3)
        self.assertEqual(wrapped_rows(label),3)
        compile_xml('<event><choice><text>'+label+'</text><event/></choice></event>')
        listing=ET.fromstring('<eventList name="L"><event/><event><item_modify><item type="fuel" min="1" max="1"/></item_modify></event></eventList>')
        full=' '.join(['abcdefghijklmnopqrs']*6)  # V62: six rows fill a lone choice; its hint row overflows.
        compile_xml('<event><choice><text>'+label+'</text><event load="L"/></choice></event>',{('eventList','L'):('fixture',listing)})
        with self.assertRaisesRegex(Unsupported,'wrapped row'):
            compile_xml('<event><choice><text>'+full+'</text><event load="L"/></choice></event>',{('eventList','L'):('fixture',listing)})
        #Effects only behind a later decision do not consume the current label's hint row.
        compile_xml('<event><choice><text>'+label+'</text><event><choice><text>later</text><event><damage amount="-1"/></event></choice></event></choice></event>')
        self.assertEqual(wrapped_rows('x'*76),2)

    def test_missing_text(self):
        with self.assertRaisesRegex(Unsupported,'missing text'):
            compile_xml('<event><text id="missing"/></event>')

    def test_unknown_effects_and_attributes(self):
        bad=['<event><ship/></event>','<event><augment name="X"/></event>',
             '<event chance="2"/>','<event><item_modify><item type="unknown" min="1" max="1"/></item_modify></event>',
             '<event><damage amount="1" system="hacking"/></event>',
             '<event><choice req="UNKNOWN_REQUIREMENT"><text>?</text><event/></choice></event>',
             '<event><autoReward level="HIGH">unknown_reward</autoReward></event>',
             '<event><text><damage amount="3"/></text></event>',
             '<event><item_modify><item type="fuel" min="1" max="1"><ship/></item></item_modify></event>',
             '<event unique="maybe"/>','<event><choice hidden="perhaps"><text>?</text><event/></choice></event>']
        for xml in bad:
            with self.subTest(xml=xml), self.assertRaises(Unsupported):compile_xml(xml)

    def test_invalid_ranges_and_malformed_choice(self):
        for xml in ['<event><item_modify><item type="fuel" min="3" max="2"/></item_modify></event>',
            '<event><item_modify><item type="fuel" min="-2" max="2"/></item_modify></event>',
            '<event><item_modify><item type="fuel" min="NaN" max="2"/></item_modify></event>',
            '<event><choice><text>missing</text></choice></event>',
            '<event load="X"><text>mixed</text></event>']:
            with self.subTest(xml=xml), self.assertRaises(Unsupported):compile_xml(xml)

    def test_bounds_fail_without_truncation(self):
        bad=['<event><text>'+('x'*1601)+'</text></event>',
             '<event>'+('<choice><text>ok</text><event/></choice>'*7)+'</event>',
             '<event><choice><text>'+('x'*229)+'</text><event/></choice></event>']
        deep='<event/>'
        for _ in range(17):deep='<event><choice><text>next</text>'+deep+'</choice></event>'
        for xml in bad+[deep]:
            with self.subTest(length=len(xml)), self.assertRaises(Unsupported):compile_xml(xml)

    def ship_definitions(self, extra="", pool="AUTO_BASIC", dead=""):
        return {('ship','S'):('fixture',ET.fromstring('<ship name="S" auto_blueprint="P"><destroyed><text>Destroyed</text><autoReward level="LOW">standard</autoReward></destroyed>'+dead+extra+'</ship>')),
                ('blueprintList','P'):('fixture',ET.fromstring('<blueprintList name="P"><name>'+pool+'</name></blueprintList>'))}

    def test_nonhostile_ship_and_branch_context(self):
        c,n=compile_xml('<event><ship load="S" hostile="false"/><choice><text>Fight</text><event><ship hostile="true"/></event></choice><choice><text>Leave</text><event/></choice></event>',self.ship_definitions())
        self.assertEqual((c.nodes[n]['ship'],c.nodes[n]['hostile']),(0,0))
        choices=c.choices[c.nodes[n]['first_choice']:]
        self.assertEqual(c.nodes[choices[0]['next']]['hostile'],1)
        self.assertEqual(c.nodes[choices[1]['next']]['hostile'],-1)
        self.assertEqual(c.ships[0]['dead_crew'],-1)
        self.assertEqual(c.enemies,['ENEMY_SCOUT'])
        self.assertEqual(c.ship_blueprints,['AUTO_BASIC'])

    def test_ship_all_outcomes_preserved(self):
        c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(dead='<deadCrew><text>Captured</text><autoReward level="HIGH">standard</autoReward><choice><text>Continue</text><event><autoReward level="MED">fuel</autoReward></event></choice></deadCrew>'))
        ship=c.ships[0]
        self.assertEqual(c.nodes[ship['destroyed']]['text'],'Destroyed')
        captured=c.nodes[ship['dead_crew']]
        self.assertEqual(captured['text'],'Captured')
        self.assertEqual(captured['choice_count'],1)
        rewards=c.effects[captured['first_effect']:captured['first_effect']+captured['effect_count']]
        self.assertIn(dict(kind=1,min=30,max=45),rewards)

    def test_ship_unsupported_outcome_rejects_root(self):
        for extra in ('<surrender/>','<escape/>','<gotaway/>','<deadCrew><weapon name="X"/></deadCrew>','<destroyed/>'):
            with self.subTest(extra=extra),self.assertRaises(Unsupported):
                compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=extra))

    def test_ship_missing_and_unknown_pool_rejected(self):
        for pool in ('UNKNOWN',''):
            with self.subTest(pool=pool),self.assertRaises(Unsupported):
                compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(pool=pool))
        with self.assertRaises(Unsupported):
            compile_xml('<event><ship load="S" hostile="true"/></event>',{})

    def test_ship_context_never_leaks_across_branches(self):
        with self.assertRaisesRegex(Unsupported,'without loaded'):
            compile_xml('<event><choice><text>One</text><event><ship load="S" hostile="false"/></event></choice><choice><text>Two</text><event><ship hostile="true"/></event></choice></event>',self.ship_definitions())
        for xml in ('<event><ship hostile="true"/></event>', '<event><ship load="S"/></event>', '<event><ship load="S" hostile="yes"/></event>', '<event><ship load="S" hostile="false"/><ship hostile="true"/></event>'):
            with self.subTest(xml=xml),self.assertRaises(Unsupported):compile_xml(xml,self.ship_definitions())

    def test_ship_loaded_callback_and_cycle(self):
        defs=self.ship_definitions()
        defs['ship','S']=('fixture',ET.fromstring('<ship name="S" auto_blueprint="P"><destroyed load="D"/></ship>'))
        defs['event','D']=('fixture',ET.fromstring('<event name="D"><text>Loaded</text></event>'))
        c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',defs)
        self.assertEqual(c.nodes[c.ships[0]['destroyed']]['text'],'Loaded')
        defs['event','D']=('fixture',ET.fromstring('<event name="D"><ship load="S" hostile="true"/></event>'))
        with self.assertRaisesRegex(Unsupported,'cyclic ship'):
            compile_xml('<event><ship load="S" hostile="true"/></event>',defs)

    def test_escape_valid_contract(self):
        extra='<escape timer="40" min="22" max="22"><text>Warning</text><ship hostile="true"/></escape><gotaway><text>Gone</text><modifyPursuit amount="1"/></gotaway>'
        c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=extra))
        ship=c.ships[0]
        self.assertEqual((ship['escape_timer'],ship['escape_min'],ship['escape_max']),(40,22,22))
        self.assertEqual(c.nodes[ship['escape_node']]['text'],'Warning')
        self.assertEqual(c.nodes[ship['escape_node']]['ship'],-1)
        self.assertEqual(c.nodes[ship['gotaway']]['pursuit'],1)
        self.assertEqual(c.nodes[ship['gotaway']]['effect_count'],0)

    def test_escape_rejects_unhandled_semantics(self):
        warning='<text>Warning</text><ship hostile="true"/>'
        valid='<escape timer="40" min="22" max="22">'+warning+'</escape><gotaway><text>Gone</text></gotaway>'
        malformed=[valid.replace('timer="40"','timer="0"'),valid.replace('timer="40"','timer="3001"'),valid.replace('timer="40"','chance="1.5"'),valid.replace('min="22"','min="23"'),valid.replace('max="22"','max="1001"'),valid.replace('min="22"','min="-1"'),valid.replace(warning,warning+'<modifyPursuit amount="1"/>'),valid.replace('hostile="true"','hostile="false"'),valid.replace('hostile="true"','hostile="true" load="S"'),valid.replace('<text>Warning</text>','<text load="LIST"/>'),valid+'<gotaway/>',valid+'<escape/>',valid.replace('<escape timer="40" min="22" max="22">'+warning+'</escape>',''),valid.replace('<text>Gone</text>','<weapon name="X"/>')]
        for extra in malformed:
            with self.subTest(extra=extra),self.assertRaises(Unsupported):
                compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=extra))

    def test_pursuit_rejects_malformed_or_unsupported(self):
        for effect in ('<modifyPursuit amount="0"/>','<modifyPursuit amount="-4"/>','<modifyPursuit amount="11"/>','<modifyPursuit/>','<modifyPursuit amount="1" extra="x"/>','<modifyPursuit amount="1"><text>x</text></modifyPursuit>','<modifyPursuit amount="1">x</modifyPursuit>','<modifyPursuit amount="1"/><modifyPursuit amount="1"/>'):
            with self.subTest(effect=effect),self.assertRaises(Unsupported):compile_xml('<event>'+effect+'</event>')
        c,n=compile_xml('<event><modifyPursuit amount="10"/></event>')
        c2,n2=compile_xml('<event><modifyPursuit amount="-2"/></event>');self.assertEqual(c2.nodes[n2]['pursuit'],-2)  # V61 original fleet delay
        self.assertEqual(c.nodes[n]['pursuit'],10)
        self.assertEqual(c.effects,[])

    def test_warning_local_nine_intros_and_callbacks(self):
        cat=json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8'))
        self.assertEqual(cat['roots'][8]['id'],'AUTO_WARNING')
        node=cat['nodes'][cat['roots'][8]['node']]
        self.assertEqual(node['alt_count'],9)
        alternatives=cat['alternatives'][node['first_alt']:node['first_alt']+9]
        self.assertEqual(len(set(cat['nodes'][n]['text'] for n in alternatives)),9)
        self.assertEqual(set(cat['nodes'][n]['ship'] for n in alternatives),{2})
        ship=cat['ships'][2]
        self.assertEqual((ship['escape_timer'],ship['escape_min'],ship['escape_max']),(40,22,22))
        self.assertEqual(cat['nodes'][ship['gotaway']]['pursuit'],1)
        self.assertEqual(cat['nodes'][ship['gotaway']]['effect_count'],0)
        self.assertEqual(cat['nodes'][ship['destroyed']]['pursuit'],0)
        self.assertEqual(cat['nodes'][ship['escape_node']]['effect_count'],0)

    def surrender_extra(self, attrs='min="3" max="4" chance="0.5"', accept='<ship hostile="false"/><autoReward level="LOW">standard</autoReward>', refusal=''):
        return '<surrender '+attrs+'><text>Offer</text><choice><text>Accept</text><event>'+accept+'</event></choice><choice><text>Refuse</text><event>'+refusal+'</event></choice></surrender>'

    def test_weighted_text_duplicates_preserved_at_expanded_bound(self):
        for count in (23,32):
            listing=ET.fromstring('<textList name="WEIGHTED">'+('<text id="a"/>'*7)+('<text id="b"/>'*(count-7))+'</textList>')
            c,n=compile_xml('<event><text load="WEIGHTED"/><item_modify><item type="fuel" min="1" max="1"/></item_modify></event>',{('textList','WEIGHTED'):('fixture',listing)},{'a':'First','b':'Second'})
            node=c.nodes[n];self.assertEqual(node['alt_count'],count)
            choices=c.alternatives[node['first_alt']:node['first_alt']+count]
            self.assertEqual(sum(c.nodes[x]['text']=='First' for x in choices),7)
            self.assertEqual(sum(c.nodes[x]['text']=='Second' for x in choices),count-7)
            self.assertTrue(all(c.nodes[x]['effect_count']==1 for x in choices))
            self.assertEqual(len(set(choices)),count)
        listing=ET.fromstring('<textList name="WEIGHTED">'+('<text id="a"/>'*33)+'</textList>')
        with self.assertRaisesRegex(Unsupported,'invalid text list'):
            compile_xml('<event><text load="WEIGHTED"/></event>',{('textList','WEIGHTED'):('fixture',listing)},{'a':'First'})

    def test_surrender_preserves_accept_refuse_and_probability(self):
        for chance, expected in (("0",0),("0.5",5000),("0.0123",123),("1.0000",10000)):
            c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=self.surrender_extra(attrs='min="0" max="4" chance="'+chance+'"')))
            ship=c.ships[0];self.assertEqual((ship['surrender_min'],ship['surrender_max'],ship['surrender_chance']),(0,4,expected))
            node=c.nodes[ship['surrender_node']];self.assertEqual(node['choice_count'],2)
            accept,refuse=c.choices[node['first_choice']:node['first_choice']+2]
            self.assertEqual(c.nodes[accept['next']]['hostile'],0)
            self.assertEqual(c.nodes[accept['next']]['effect_count'],3)
            self.assertEqual((c.nodes[refuse['next']]['hostile'],c.nodes[refuse['next']]['effect_count']),(-1,0))

    def test_surrender_rejects_bounds_probability_and_duplicates(self):
        bad=('min="5" max="4"','min="-1" max="4"','min="1" max="1001"','min="1"','min="1" max="3" extra="x"')
        bad+=tuple('min="1" max="4" chance="'+n+'"' for n in ('NaN','inf','-0.1','1.1','0.12345','1e-1',''))
        for attrs in bad:
            with self.subTest(attrs=attrs),self.assertRaises(Unsupported):
                compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=self.surrender_extra(attrs=attrs)))
        with self.assertRaisesRegex(Unsupported,'duplicate surrender'):
            compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=self.surrender_extra()*2))

    def test_surrender_rejects_unhandled_branch_semantics(self):
        bad=[self.surrender_extra(refusal='<autoReward level="LOW">standard</autoReward>'),
             self.surrender_extra(refusal='<modifyPursuit amount="1"/>'),
             self.surrender_extra(refusal='<ship load="S" hostile="true"/>'),
             self.surrender_extra(accept='<text>Ambiguous acceptance</text>'),
             self.surrender_extra(accept='<ship hostile="false"/><crewMember amount="2"/>'),
             self.surrender_extra(accept='<ship hostile="false"/><choice><text>Attack</text><event><ship hostile="true"/></event></choice>')]
        for extra in bad:
            with self.subTest(extra=extra),self.assertRaises(Unsupported):
                compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=extra))
        # A reward in a preceding node must not leak into an eventual refusal.
        extra=self.surrender_extra().replace('<text>Offer</text>','<text>Offer</text><autoReward level="LOW">standard</autoReward>')
        with self.assertRaisesRegex(Unsupported,'refusal'):
            compile_xml('<event><ship load="S" hostile="true"/></event>',self.ship_definitions(extra=extra))

    def test_surrender_loaded_nested_and_all_random_branches(self):
        defs=self.ship_definitions(extra='<surrender min="4" max="4" load="DIALOG"/>')
        defs['event','DIALOG']=('fixture',ET.fromstring('<event name="DIALOG"><text>Ask</text><choice><text>Continue</text><event>'+self.surrender_extra().split('>',1)[1].rsplit('</surrender>',1)[0]+'</event></choice></event>'))
        c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',defs)
        self.assertEqual(c.ships[0]['surrender_chance'],10000)
        first=c.nodes[c.ships[0]['surrender_node']];self.assertEqual(first['choice_count'],1)
        nested=c.nodes[c.choices[first['first_choice']]['next']];self.assertEqual(nested['choice_count'],2)
        defs['event','DIALOG']=('fixture',ET.fromstring('<event name="DIALOG"><choice><text>Choose</text><event load="RANDOM"/></choice></event>'))
        defs['eventList','RANDOM']=('fixture',ET.fromstring('<eventList name="RANDOM"><event><ship hostile="false"/></event><event><autoReward level="LOW">standard</autoReward></event></eventList>'))
        with self.assertRaisesRegex(Unsupported,'refusal'):
            compile_xml('<event><ship load="S" hostile="true"/></event>',defs)

    def test_real_engi_surrender_compiles_but_remains_out_of_pool(self):
        archive=Path(r'C:\Program Files (x86)\Steam\steamapps\common\FTL Faster Than Light\ftl.dat')
        if not archive.exists():self.skipTest('Owner archive unavailable; synthetic closure tests still run')
        definitions,strings,_=read_archive(PkgArchive(archive));c=Compiler(definitions,strings)
        c.compile(c.resolve('event','ENGI_UNLOCK_2FAKE'),('ENGI_UNLOCK_2FAKE',))
        ship=c.ships[0]
        self.assertEqual(ship['id'],'REBEL_ENGI_UNLOCK_2FAKE')
        self.assertEqual((ship['surrender_min'],ship['surrender_max'],ship['surrender_chance']),(4,4,10000))
        self.assertEqual((ship['escape_timer'],ship['escape_min'],ship['escape_max']),(40,18,18))
        self.assertEqual(c.ship_blueprints,['REBEL_FAT','REBEL_SKINNY'])
        self.assertGreaterEqual(ship['destroyed'],0);self.assertGreaterEqual(ship['dead_crew'],0);self.assertGreaterEqual(ship['gotaway'],0)
        self.assertNotIn('ENGI_UNLOCK_2FAKE',SELECTED)
        self.assertEqual(len(SELECTED),12)

    def test_local_ship_pool_and_result_contract(self):
        cat=json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8'))
        # The V29 reviewed roots come first; V53 sector-list ships follow.
        self.assertEqual(cat['ship_blueprints'][:10],['AUTO_BASIC','AUTO_ASSAULT','REBEL_FAT','REBEL_SKINNY','AUTO_BASIC','AUTO_ASSAULT','AUTO_BASIC','AUTO_ASSAULT','REBEL_FAT','REBEL_SKINNY'])
        self.assertEqual(cat['enemies'][:10],['ENEMY_SCOUT','ENEMY_ASSAULT','ENEMY_REBEL_FAT','ENEMY_REBEL','ENEMY_SCOUT','ENEMY_ASSAULT','ENEMY_SCOUT','ENEMY_ASSAULT','ENEMY_REBEL_FAT','ENEMY_REBEL'])
        self.assertEqual([x['id'] for x in cat['ships']][:5],['REBEL_AUTO_REFUEL','SQUAT_REFUEL_STATION','REBEL_AUTO_WARNING','REBEL_AUTO_CIVILIAN','REBEL'])
        self.assertTrue(all(e.startswith('ENEMY_') for e in cat['enemies']))
        self.assertEqual(cat['ships'][0]['dead_crew'],-1)
        self.assertGreaterEqual(cat['ships'][1]['dead_crew'],0)
        for ship in cat['ships'][:2]:
            for key in ('destroyed','dead_crew'):
                if ship[key]>=0:self.assertEqual(cat['nodes'][ship[key]]['choice_count'],1)

    def test_pending_reward_schema_and_strict_input(self):
        for xml, expected in (( '<autoReward level="LOW">weapon</autoReward>', (5,0,6)),
                              ('<crewMember amount="1"/>', (6,0,5)),
                              ('<crewMember amount="1" class="engi"/>', (6,1,1)),
                              ('<crewMember amount="1" class="crystal"/>', (6,6,6))):
            c,n=compile_xml('<event>'+xml+'</event>')
            self.assertEqual(tuple(c.effects[0].values()),expected)
        invalid=('<crewMember/>','<crewMember amount="0"/>','<crewMember amount="2"/>',
                 '<crewMember amount="-1"/>','<crewMember amount="1" class="unknown"/>',
                 '<crewMember amount="1" name="Joe"/>','<crewMember amount="1">human</crewMember>',
                 '<crewMember amount="1"><text>x</text></crewMember>',
                 '<autoReward level="HIGH">unknown_reward</autoReward>',
                 '<autoReward level="RANDOM">unknown</autoReward>')
        for effect in invalid:
            with self.subTest(effect=effect),self.assertRaises(Unsupported):compile_xml('<event>'+effect+'</event>')

    def test_pending_object_limit_covers_all_random_branches(self):
        crew='<crewMember amount="1"/>';weapon='<autoReward level="LOW">weapon</autoReward>'
        for effects in (crew+crew,weapon+weapon,crew+weapon):
            with self.assertRaisesRegex(Unsupported,'multiple pending'):
                compile_xml('<event>'+effects+'</event>')
            listing=ET.fromstring('<eventList name="L"><event>'+weapon+'</event><event>'+effects+'</event></eventList>')
            with self.assertRaisesRegex(Unsupported,'multiple pending'):
                compile_xml('<event load="L"/>',{('eventList','L'):('fixture',listing)})
        # A later explicit choice is a separate action after pending reward resolution.
        compile_xml('<event>'+crew+'<choice><text>Later</text><event>'+weapon+'</event></choice></event>')

    def test_real_reward_closures_keep_every_branch(self):
        cat=json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8'))
        self.assertEqual(cat['schema'],7)
        self.assertEqual([r['id'] for r in cat['roots'][9:11]],['FIND_WEAPON','AUTO_CIVILIAN'])
        find=cat['nodes'][cat['roots'][9]['node']]
        self.assertEqual(find['alt_count'],6)
        for index in cat['alternatives'][find['first_alt']:find['first_alt']+6]:
            node=cat['nodes'][index]
            self.assertEqual(cat['effects'][node['first_effect']],dict(kind=5,min=0,max=6))
        root=cat['nodes'][cat['roots'][10]['node']]
        self.assertEqual(root['choice_count'],2)
        ship=cat['ships'][3];self.assertEqual(ship['id'],'REBEL_AUTO_CIVILIAN')
        self.assertEqual((ship['escape_node'],ship['surrender_node']),(-1,-1))
        for callback in ('destroyed','dead_crew'):
            outcome=cat['nodes'][ship[callback]]
            result=cat['nodes'][cat['choices'][outcome['first_choice']]['next']]
            self.assertEqual(result['alt_count'],6)
            alts=cat['alternatives'][result['first_alt']:result['first_alt']+6]
            recruit=cat['nodes'][alts[0]];self.assertEqual(recruit['choice_count'],2)
            yes,no=cat['choices'][recruit['first_choice']:recruit['first_choice']+2]
            accepted=cat['nodes'][yes['next']]
            self.assertEqual(cat['effects'][accepted['first_effect']],dict(kind=6,min=0,max=5))
            self.assertEqual(cat['nodes'][no['next']]['effect_count'],0)
            weapon=cat['nodes'][alts[3]]
            item=cat['nodes'][cat['choices'][weapon['first_choice']]['next']]
            self.assertEqual(cat['effects'][item['first_effect']],dict(kind=5,min=0,max=6))
            repair=cat['nodes'][alts[4]]
            self.assertEqual(cat['effects'][repair['first_effect']],dict(kind=4,min=5,max=5))
            self.assertEqual(cat['nodes'][alts[5]]['effect_count'],0)

    def test_new_resource_reward_families(self):
        for name,level,expected in [('stuff','RANDOM',[(0,1,4),(2,1,4),(3,1,3)]),
                                    ('fuel','HIGH',[(0,5,8),(1,20,30)]),
                                    ('scrap_only','LOW',[(1,10,20)])]:
            c,n=compile_xml('<event><autoReward level="'+level+'">'+name+'</autoReward></event>')
            self.assertEqual([tuple(e.values()) for e in c.effects],expected)
        for xml in ('<autoReward level="LOW">unknown_reward</autoReward>',
                    '<autoReward level="HIGH">unknown_kind</autoReward>',
                    '<autoReward level="RANDOM" extra="1">stuff</autoReward>'):
            with self.assertRaises(Unsupported):compile_xml('<event>'+xml+'</event>')

    def test_escape_probability_defaults_and_loaded_closure(self):
        definitions=self.ship_definitions('<escape chance="0.5" min="3" max="4" load="WARNING"/>')
        definitions[('event','WARNING')]=('fixture',ET.fromstring('<event name="WARNING"><text>Leaving</text><ship hostile="true"/></event>'))
        c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',definitions)
        ship=c.ships[0]
        self.assertEqual((ship['escape_timer'],ship['escape_chance'],ship['gotaway']),(40,5000,-1))
        for chance,value in [('0',0),('1',10000),('0.1234',1234)]:
            definitions=self.ship_definitions('<escape chance="'+chance+'" min="3" max="4"><text>Leaving</text><ship hostile="true"/></escape>')
            c,n=compile_xml('<event><ship load="S" hostile="true"/></event>',definitions)
            self.assertEqual(c.ships[0]['escape_chance'],value)
        for chance in ('-0.5','1.1','0.00001','NaN','', '0.5junk'):
            definitions=self.ship_definitions('<escape chance="'+chance+'" min="3" max="4"><text>Leaving</text><ship hostile="true"/></escape>')
            with self.assertRaises(Unsupported):compile_xml('<event><ship load="S" hostile="true"/></event>',definitions)

    def test_escape_loaded_warning_stays_fail_closed(self):
        for body in ('<text>Leaving</text><ship hostile="true"/><autoReward level="LOW">standard</autoReward>',
                     '<text>Leaving</text><ship hostile="true"/><choice><text>Wait</text><event/></choice>',
                     '<text>Leaving</text><ship hostile="false"/>',
                     '<text>Leaving</text><ship hostile="true"/><modifyPursuit amount="1"/>'):
            definitions=self.ship_definitions('<escape chance="0.5" min="3" max="4" load="WARNING"/>')
            definitions[('event','WARNING')]=('fixture',ET.fromstring('<event name="WARNING">'+body+'</event>'))
            with self.assertRaises(Unsupported):compile_xml('<event><ship load="S" hostile="true"/></event>',definitions)

    def test_secret_sector_ends_a_closed_event(self):
        c,n=compile_xml('<event><text>Coordinates</text><secretSector/></event>')
        self.assertEqual(c.effects,[dict(kind=18,min=0,max=0)])
        for xml in ('<event><secretSector/><choice><text>Go</text><event/></choice></event>',
                    '<event><secretSector/><secretSector/></event>','<event><secretSector x="1"/></event>',
                    '<event><secretSector><text>no</text></secretSector></event>'):
            with self.subTest(xml=xml),self.assertRaises(Unsupported):compile_xml(xml)
        definitions=self.ship_definitions('')
        with self.assertRaises(Unsupported):compile_xml('<event><ship load="S" hostile="false"/><secretSector/></event>',definitions)

    def test_installed_archive_reproduces_complete_catalog(self):
        archive=Path(r'C:\Program Files (x86)\Steam\steamapps\common\FTL Faster Than Light\ftl.dat')
        if not archive.exists():self.skipTest('Owner archive unavailable; local closure tests still run')
        catalog,coverage=build(PkgArchive(archive))
        self.assertEqual(catalog,json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8')))
        self.assertEqual(coverage,json.loads((ROOT/'generated/event_coverage.json').read_text(encoding='utf-8')))
        self.assertEqual(header(catalog),(ROOT/'generated/event_catalog.h').read_text(encoding='utf-8'))

    def test_generic_rebel_complete_weighted_closure(self):
        cat=json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8'))
        self.assertEqual(cat['roots'][11]['id'],'REBEL')
        ship=cat['ships'][4]
        self.assertEqual((ship['id'],ship['surrender_min'],ship['surrender_max'],ship['surrender_chance']),('REBEL',2,3,5000))
        self.assertEqual((ship['escape_min'],ship['escape_max'],ship['escape_chance'],ship['escape_timer'],ship['gotaway']),(3,4,5000,40,-1))
        intro=cat['nodes'][cat['roots'][11]['node']];self.assertEqual(intro['alt_count'],10)
        offer=cat['nodes'][ship['surrender_node']];self.assertEqual(offer['alt_count'],23)
        variants=cat['alternatives'][offer['first_alt']:offer['first_alt']+23]
        self.assertEqual(len(set(cat['nodes'][n]['text'] for n in variants)),14)
        for n in variants:
            node=cat['nodes'][n];self.assertEqual(node['choice_count'],2)
            yes,no=cat['choices'][node['first_choice']:node['first_choice']+2]
            accepted=cat['nodes'][yes['next']];refused=cat['nodes'][no['next']]
            self.assertEqual(accepted['hostile'],0);self.assertEqual(accepted['effect_count'],3)
            self.assertEqual((refused['hostile'],refused['effect_count']),(-1,0))
        dead=cat['nodes'][ship['dead_crew']];self.assertEqual(dead['alt_count'],9)
        rewards=[]
        for n in cat['alternatives'][dead['first_alt']:dead['first_alt']+9]:
            node=cat['nodes'][n]
            rewards.append([tuple(e.values()) for e in cat['effects'][node['first_effect']:node['first_effect']+node['effect_count']]])
        self.assertEqual(rewards, [[(0,1,3),(2,1,3),(1,20,30)]]*3 + [[(0,1,3),(2,1,3),(1,30,45)]]*2 + [[(0,5,8),(1,20,30)]]*2 + [[(6,0,5),(1,10,20)],[(5,0,6)]])

    def test_local_catalog_integrity_and_closure(self):
        catalog=json.loads((ROOT/'generated/event_catalog.json').read_text(encoding='utf-8'))
        coverage=json.loads((ROOT/'generated/event_coverage.json').read_text(encoding='utf-8'))
        self.assertEqual(tuple(r['id'] for r in catalog['roots'])[:12],SELECTED)
        self.assertEqual((ROOT/'generated/event_catalog.h').read_text(encoding='utf-8'),header(catalog))
        self.assertEqual(sum(e['status']=='enabled' for e in coverage['entries']),coverage['enabled'])
        self.assertGreaterEqual(coverage['enabled'],12)
        self.assertTrue(all(e['reason'] for e in coverage['entries']))
        reached=set()
        def visit(index,path=()):
            self.assertNotIn(index,path);self.assertLess(len(path),16)
            self.assertTrue(0<=index<len(catalog['nodes']));reached.add(index)
            n=catalog['nodes'][index]
            self.assertLessEqual(len(n['text']),1600);self.assertLessEqual(n['choice_count'],6)
            self.assertLessEqual(n['first_effect']+n['effect_count'],len(catalog['effects']))
            self.assertLessEqual(n['first_choice']+n['choice_count'],len(catalog['choices']))
            self.assertLessEqual(n['first_alt']+n['alt_count'],len(catalog['alternatives']))
            for e in catalog['effects'][n['first_effect']:n['first_effect']+n['effect_count']]:
                self.assertIn(e['kind'],range(19));self.assertTrue(e['min']<=e['max'] or e['kind'] in (7,8,16,17))
                if e['kind']==12:visit(e['min'],path+(index,))  # V61 quest beacon target
            for c in catalog['choices'][n['first_choice']:n['first_choice']+n['choice_count']]:
                self.assertLessEqual(len(c['text']),228);visit(c['next'],path+(index,))
            for a in catalog['alternatives'][n['first_alt']:n['first_alt']+n['alt_count']]:visit(a,path+(index,))
        for r in catalog['roots']:visit(r['node'])
        for ship in catalog['ships']:
            visit(ship['destroyed'])
            for key in ('dead_crew','escape_node','gotaway','surrender_node'):
                if ship[key] >= 0:visit(ship[key])
        self.assertEqual(len(reached),len(catalog['nodes']))

if __name__=='__main__':unittest.main()
