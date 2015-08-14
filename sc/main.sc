/*
This file is part of AlgAudio.

AlgAudio, Copyright (C) 2015 CeTA - Audiovisual Technology Center

AlgAudio is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

AlgAudio is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with AlgAudio.  If not, see <http://www.gnu.org/licenses/>.
*/


/*
	Synths are stored in a quite an unobvious structure. It would be much more
	elegant to write a class that manages a single module instance, but it's too
	hard to hook up to the class library without planting custom files in the
	system. So, instead, a following structure is used:

	~minstances is a dict (int -> module_instance), which is simply a set of all
		created instances, indexed by their synth_id (serverside). All messages
		from algaudio identify synths by these ids, so such dict is convenient.

	A module_instance is an array of exactly 3 elements.
	First one, is the group. The synth and it's forks are gouped together in
		order to simplify toposort and ordering.
	The second element is the synth instance itself. This is where param sets
		are passed to. It is build basing on a module-defined synthdef.
	The third element is a dict (string -> fork_pair). It is a collection
		of fork pairs indexed by outlet name. If an outlet has 0 or 1
		connection, it should not be present in this dict. Otherwise it should have
		exactly one fork pair bound.

	A fork pair is a 2-element array. The first element is a bus id, the second
		is a fork synth instance. The bus is the one used to connect the module
		synth with the fork.

*/


// This method estabilishes two-way osc connection
OSCdef.new( 'hello', {
		arg msg, time, addr, recvPort;
		[msg, time, addr, recvPort].postln;
		// This is when the ~addr is stored.
		~addr = addr;
		~minstances = Dictionary.new(0);
		~buses = Dictionary.new(0);
		"Hello World!".postln;
		~addr.sendMsg("/algaudio/reply",msg[msg.size-1]);
	}, '/algaudioSC/hello'
).postln;


OSCdef.new( 'installtemplate', {
		arg msg;
		var command, f;
		if((msg.size < 2),{
			"Invalid message".postln;
		},{
			("Installing template " ++ msg[1]).postln;
			command = "SynthDef.new('aa/" ++ msg[1] ++ "', {" ++ msg[2] ++ "}).add;";
			//command.postln;
			f = command.compile();
			f.value();
		});
		~addr.sendMsg("/algaudio/reply", "aaaaaa", msg[msg.size-1]);
	}, '/algaudioSC/installtemplate'
).postln;

// A helper method for notifying the app if starting the server succeeded.
OSCdef.new( 'boothelper', {
		arg msg;
		s.options.device = msg[1].asString;
		if((msg[2].asInt == 1),{
			"Server will start in Supernova mode".postln;
			Server.supernova;
		},{
			"Server will start in SCSynth mode".postln;
			Server.scsynth;
		});
		s.waitForBoot({
			~addr.sendMsg("/algaudio/reply", 1, msg[msg.size-1]);
		}, 35, {
			~addr.sendMsg("/algaudio/reply", 0, msg[msg.size-1]);
		});
	}, '/algaudioSC/boothelper'
).postln;

// 1 argument: the template name
// reply value: instance id
OSCdef.new( 'newinstance', {
		arg msg;
		var name = "aa/" ++ msg[1];
		var group = Group.new();
		var synth = Synth.new(name, [], group);
		var forkdict = Dictionary.new(0);
		var id = synth.nodeID;
		("Created new \"" ++ name ++ "\" instance " ++ id.asString ).postln;
		~minstances.add( id -> [group, synth, forkdict]);
		~addr.sendMsg("/algaudio/reply", id, msg[msg.size-1]);
	}, '/algaudioSC/newinstance'
).postln;

// args: the template name + a list of params and values
// reply value: instance id
OSCdef.new( 'newinstanceparams', {
		arg msg;
		var name = "aa/" ++ msg[1];
		var params = msg[2..(msg.size-2)];
		var group = Group.new();
		var synth = Synth.new(name, params, group);
		var forkdict = Dictionary.new(0);
		var id = synth.nodeID;
		("Created new \"" ++ name ++ "\" instance " ++ id.asString ++ " with " ++ params.asString ).postln;
		~minstances.add( id -> [group, synth, forkdict]);
		~addr.sendMsg("/algaudio/reply", id, msg[msg.size-1]);
	}, '/algaudioSC/newinstanceparams'
).postln;

// 1 argument: the template id
// reply value: instance id
OSCdef.new( 'removeinstance', {
		arg msg;
		var id = msg[1];
		("Removing instance \"" ++ id.asString ++ "\".").postln;
		~minstances[id][1].free;
		~minstances[id][0].free;
		~minstances.removeAt( id );
		~addr.sendMsg("/algaudio/reply", msg[msg.size-1]);
	}, '/algaudioSC/removeinstance'
).postln;

// reply value: bus instance id
OSCdef.new( 'newbus', {
		arg msg;
		var newbus = Bus.audio(s,1);
		var id = newbus.index;
		("Creating new bus " ++ id.asString).postln;
		~buses.add( id -> newbus); // TODO: default args
		~addr.sendMsg("/algaudio/reply", id, msg[msg.size-1]);
	}, '/algaudioSC/newbus'
).postln;

// Args: instance id, param name, value
OSCdef.new( 'setparam', {
		arg msg;
		if((msg[1] == -1),{
			("Not setting param " ++ msg[2].asString ++ " of " ++ msg[1].asString ++ " to " ++ msg[3] ++ ", because it's not a valid instance!").postln;
		});
		//("Setting param " ++ msg[2].asString ++ " of " ++ msg[1].asString ++ " to " ++ msg[3]).postln;
		~minstances[msg[1]][1].set(
			msg[2].asString,
			msg[3]
		);
	}, '/algaudioSC/setparam'
).postln;

// Args: instance id, param name, values
OSCdef.new( 'setparamlist', {
		arg msg;
		var list = msg[3..(msg.size-2)];
		if((msg[1] == -1),{
			("Not setting param " ++ msg[2].asString ++ " of " ++ msg[1].asString ++ " to a list, because it's not a valid instance!").postln;
		});
		//("Setting param " ++ msg[2].asString ++ " of " ++ msg[1].asString ++ " to " ++ list).postln;
		~minstances[msg[1]][1].set(
			msg[2].asString,
			list
		);
	}, '/algaudioSC/setparamlist'
).postln;

// Args: instance id, outlet id, bus id
OSCdef.new( 'connectoutlet', {
		arg msg;
		var synthid = msg[1];
		var targets = msg[3..(msg.size-2)];
		var target = msg[3];
		var outlet = msg[2].asString;
		var count = targets.size;
		var fork, bus, forkpair;
		("Connecting outlet " ++ synthid.asString ++ "/" ++ msg[2].asString ++ " to bus " ++ targets.asString ++ " count " ++ count).postln;
		// Free the fork synth and bus, if they exists.
		forkpair = ~minstances[synthid][2][outlet];
		if((forkpair.notNil),{
			forkpair[0].free;
			forkpair[1].free;
			~minstances[synthid][2].removeAt(outlet);
		},{});

		if((count == 1), {
			// Simple case.
			// Disconnected outlet
			if((target == -1),{target = 9999999},{});
			// Then bind the synth directly to target
			~minstances[synthid][1].set( outlet, target );
		},{
			if((count <= 2),{
				// Build a new forkpair.
				bus = Bus.audio(s,1);
				( [bus.index] ++ targets).postln;
				fork = Synth.new("builtin/fork2", ["in", [bus.index,targets[0],targets[1]]], ~minstances[synthid][1], \addAfter );
				fork.postln;
				~minstances[synthid][2][outlet] = [bus,fork];
				// Then bind the synth to the fork
				~minstances[synthid][1].set( outlet, bus.index );
			},{
				("ERROR: Failed to connect, count is over maximum allowed").postln;
			});
		});
	}, '/algaudioSC/connectoutlet'
).postln;

// Args: instance id, inlet id, bus id
OSCdef.new( 'connectinlet', {
		arg msg;
		("Connecting inlet " ++ msg[1].asString ++ "/" ++ msg[2].asString ++ " to bus " ++ msg[3]).postln;
		~minstances[msg[1]][1].set(
			msg[2].asString,
			msg[3]
		);
	}, '/algaudioSC/connectinlet'
).postln;

// This is the helper method that realizes a synth ordering.
OSCdef.new( 'ordering', {
		arg msg;
		var first = msg[1];
		var all = msg[1..(msg.size-2)];
		("Applying ordering: " ++ all.asString).postln;
		~minstances[first][0].moveToHead(~minstances[first][0].group);
		for(1,msg.size-3,{ arg i;
			~minstances[all[i]][0].moveAfter(~minstances[all[i-1]][0]);
		});
	}, '/algaudioSC/ordering'
).postln;

// Helper catcher for SendReply-ies
OSCdef.new( 'sendreply', {
		arg msg;
		~addr.sendMsg("/algaudio/sendreply",msg[1],msg[2],msg[3]);
	}, '/algaudioSC/sendreply'
).postln;

// A dummy helper for listing all installed synthdefs
OSCdef.new( 'listall', {
		SynthDescLib.global.synthDescs.do { |desc|
			if(desc.def.notNil) {
				"\nSynthDef %\n".postf(desc.name.asCompileString);
				//desc.def.func.postcs;
			};
		};
	}, '/algaudioSC/listall'
).postln;


SynthDef.new("builtin/fork2",{ arg in, out1, out2;var i = In.ar(in);
	Out.ar(out1,i); Out.ar(out2,i);
}).add;
