/*******************************************************************************
 * Copyright (C) 2016 Sam Silverberg sam.silverberg@gmail.com	
 *
 * This file is part of OpenDedupe SDFS.
 *
 * OpenDedupe SDFS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * OpenDedupe SDFS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
package org.opendedup.util;

import java.io.IOException;

import com.google.gson.FieldNamingPolicy;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonObject;

public class TestFile {
	public static void main(String[] args) throws IOException {
		JsonObject obj = new JsonObject();
		obj.addProperty("poop", "wow");
		JsonObject ar = new JsonObject();
		ar.addProperty("ar1", 1);
		ar.addProperty("ar2", 2);
		obj.add("ars", ar);
		Gson gson = new GsonBuilder()
				.setPrettyPrinting()
				.serializeNulls()
				.setFieldNamingPolicy(
						FieldNamingPolicy.UPPER_CAMEL_CASE).create();
		System.out.println(gson.toJson(obj));
	}

}
